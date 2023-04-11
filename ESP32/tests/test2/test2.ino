/**
 *
 */
#undef FASTLED_ALL_PINS_HARDWARE_SPI
#undef FASTLED_ESP32_I2S
#include <FastLED.h>
#include "Esp32PcntRotaryEncoder.h"

const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const uint16_t PULSE_COUNT_CYCLE = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

#define Q_SIZE 64
QueueHandle_t xQue;

#define PIN_NEOPIXEL 27
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

/**
 *
 */
void IRAM_ATTR re_intr_hdr(void *arg) {
  log_w("int_str=0x%04X, status=0x%04X",
        PCNT.int_st.val, PCNT.status_unit[0].val);

  PCNT.int_clr.val = PCNT.int_st.val;
}

/**
 *
 */
void re_watcher(void *pvParameters) {
  Esp32PcntRotaryEncoder *re;

  re = new Esp32PcntRotaryEncoder(PIN_PULSE_CLK, PIN_PULSE_DT,
                                  PCNT_UNIT, PULSE_COUNT_CYCLE,
                                  re_intr_hdr, (void *)NULL);

  while (true) {
    int16_t d_angle = re->get();

    if ( d_angle == 0 ) {
      delay(1);
      continue;
    }
    
    log_i("> d_angle=%d", d_angle);

    portBASE_TYPE ret;

    while ( (ret = xQueueSend(xQue, (void *)&d_angle, 10)) != pdPASS ) {
      log_w("put queue failed");
      delay(1);
    }

    delay(20);
  } // while(true)
}

/**
 *
 */
void ch_color(uint8_t r, uint8_t g, uint8_t b) {
  leds[0] = CRGB(r, g, b);
  FastLED.show();
}

/**
 *
 */
void task2(void *pvParameters) {
  int16_t d_angle;
  portBASE_TYPE ret;
  
  while (true) {
    while ( (ret = xQueueReceive(xQue, (void *)&d_angle, 10)) != pdPASS ) {
      delay(1);
      // ch_color(255, 255, 255);
    }

    if ( d_angle > 0 ) {
      ch_color(255, 0, 0);
    }
    if ( d_angle < 0 ) {
      ch_color(0, 0, 255);
    }
    
    log_i("     < d_angle=   %d", d_angle);

    delay(1);
  } // while(true)
}

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(50);  // Serial Init Wait

#if 0
  //@file color.h
  typedef enum {
   // Color correction starting points

   /// typical values for SMD5050 LEDs
   ///@{
   TypicalSMD5050=0xFFB0F0 /* 255, 176, 240 */,
   TypicalLEDStrip=0xFFB0F0 /* 255, 176, 240 */,
   ///@}

   /// typical values for 8mm "pixels on a string"
   /// also for many through-hole 'T' package LEDs
   ///@{
   Typical8mmPixel=0xFFE08C /* 255, 224, 140 */,
   TypicalPixelString=0xFFE08C /* 255, 224, 140 */,
   ///@}

   /// uncorrected color
   UncorrectedColor=0xFFFFFF

   } LEDColorCorrection;
#endif
  FastLED.addLeds<WS2812B, PIN_NEOPIXEL, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(30);
  leds[0] = CRGB(255,255,255);
  FastLED.show();

  xQue = xQueueCreate(Q_SIZE, sizeof(int16_t));
  if ( xQue == NULL ) {
    log_e("xQueueCreate: failed");

    while(true) {
      delay(1);
    }
  }

  xTaskCreateUniversal(
    re_watcher,
    "re_watcher",
    0x1000,
    NULL,
    1,
    NULL,
    APP_CPU_NUM
  );

  xTaskCreateUniversal(
    task2,
    "task2",
    0x1000,
    NULL,
    1,
    NULL,
    APP_CPU_NUM
  );
}

/**
 *
 */
void loop() {
  delay(100);
}
