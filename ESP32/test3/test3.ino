/**
 *
 */
#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include "Esp32PcntRotaryEncoder.h"
#include "NetMgr.h"

// Rotary Encoder
const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const uint16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

// Queue
#define Q_SIZE 64
QueueHandle_t xQue;

// NeoPixel
const uint8_t PIN_NEOPIXEL = 27;
const uint16_t NUM_LEDS = 1;
const uint8_t LED_BRIGHTNESS = 50;
CRGB leds[NUM_LEDS];

// WiFi
const String AP_SSID_HDR = "test3";
const unsigned int WIFI_RETRY_COUNT = 10;
NetMgr *netMgr;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
const TickType_t NTP_INTERVAL = 10 * 1000; // tick == ms (?)


/**
 *
 */
void IRAM_ATTR re_intr_hdr(void *arg) {
  log_w("int_str=0x%04X, status=0x%04X",
        PCNT.int_st.val, PCNT.status_unit[0].val);

  PCNT.int_clr.val = PCNT.int_st.val;
}

mode_t netMgrMode;

/**
 *
 */
void timer_cb_ntp(TimerHandle_t xTimer) {
  log_d("%x> start netMgrMode=0x%02X", xTimer, netMgrMode);
  if ( netMgrMode != NetMgr::MODE_WIFI_ON ) {
    return;
  }

  struct tm ti; // time info
  const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  
  configTime(9 * 3600L, 0,
             NTP_SVR[0].c_str(), NTP_SVR[0].c_str(), NTP_SVR[0].c_str());

  getLocalTime(&ti);
  log_i("%04d/%02d/%02d(%s) %02d:%02d:%02d",
        ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
        day_str[ti.tm_wday].c_str(),
        ti.tm_hour, ti.tm_min, ti.tm_sec);
}

/** NetMgr task function
 *
 */
void task_net_mgr(void *pvParameters) {
  netMgr = new NetMgr(AP_SSID_HDR, WIFI_RETRY_COUNT);

  TimerHandle_t ntp_timer = xTimerCreate("NTP", 10000, pdTRUE, NULL,
                                         timer_cb_ntp);
  xTimerStart(ntp_timer, 0);
  
  while (true) { // main loop
    static mode_t prev_netMgrMode = NetMgr::MODE_NULL;

    netMgrMode = netMgr->loop();
    if ( netMgrMode != prev_netMgrMode ) {
      log_i("netMgrMode=0x%02X", netMgrMode);
      prev_netMgrMode = netMgrMode;
    }
  } // while(true)
}

/** Rotary Encoder watcher task function
 *
 */
void task_re_watcher(void *pvParameters) {
  Esp32PcntRotaryEncoder *re;

  re = new Esp32PcntRotaryEncoder(PIN_PULSE_CLK, PIN_PULSE_DT,
                                  PCNT_UNIT, PULSE_MAX,
                                  re_intr_hdr, (void *)NULL);

  while (true) {
    int16_t d_angle = re->get();

    if ( d_angle == 0 ) {
      delay(1);
      continue;
    }
    
    int16_t angle = re->angle;
    log_d("> angle=%d/%d, d_angle=%d", angle, PULSE_MAX, d_angle);

    portBASE_TYPE ret;
    // log_d("AAA");

    while ( (ret = xQueueSend(xQue, (void *)&angle, 10)) != pdPASS ) {
      log_w("put queue failed");
      delay(1);
    }

    delay(20);
  } // while(true)
}

/**
 * 
 */
unsigned long hsv2rgb(uint8_t hue, uint8_t sat, uint8_t val) {
  log_d("hue=%d, sat=%d, val=%d", hue, sat, val);


  if ( sat == 0 ) {
    unsigned long rgb = (val << 16) + (val << 8) + val;
    return rgb;
  }
  
  unsigned char region = hue / 43;
  unsigned char remainder = (hue - (region * 43)) * 6;

  unsigned char p = (val * (255 - sat)) >> 8;
  unsigned char q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
  unsigned char t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

  uint8_t r, g, b;
  switch (region) {
  case 0:
    r = val;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = val;
    b = p;
    break;
  case 2:
    r = p;
    g = val;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = val;
    break;
  case 4:
    r = t;
    g = p;
    b = val;
    break;
  default:
    r = val;
    g = p;
    b = q;
    break;
  }

  log_d("r=%d, g=%d, b=%d", r, g, b);
  unsigned long rgb = (r << 16) + (g << 8) + b;
  log_d("rgb=#%06X", rgb);
  return rgb;
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
void task1(void *pvParameters) {
  int16_t angle;
  portBASE_TYPE ret;
  
  while (true) {
    // get queue
    while ( (ret = xQueueReceive(xQue, (void *)&angle, 10)) != pdPASS ) {
      delay(1);
    }
    log_d("     < angle=   %d/%d", angle, PULSE_MAX);

    // calc color
    uint16_t hue = angle * 255 / PULSE_MAX;
    uint8_t sat = 255;
    uint8_t val = 255;

    unsigned long rgb = hsv2rgb(hue, sat, val);
    log_i("angle=%d, HSV=%d,%d,%d, rgb=#%06X", angle, hue, sat, val, rgb);

    uint8_t r = (rgb & 0xff0000) >> 16;
    uint8_t g = (rgb & 0x00ff00) >> 8;
    uint8_t b = (rgb & 0x0000ff);

    log_d("r=%d, g=%d, b=%d", r, g, b);
    ch_color(r, g, b);

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
  FastLED.setBrightness(LED_BRIGHTNESS);
  leds[0] = CRGB(255,255,255);
  FastLED.show();

  xQue = xQueueCreate(Q_SIZE, sizeof(int16_t));
  if ( xQue == NULL ) {
    log_e("xQueueCreate: failed");

    while(true) {
      delay(1);
    }
  }

  xTaskCreateUniversal(task_re_watcher, "re_watcher",
                       0x1000,
                       NULL,
                       1,
                       NULL,
                       APP_CPU_NUM
                       );

  xTaskCreateUniversal(task_net_mgr, "net_mgr",
                       (const uint32_t)0x1000 /* stack size */,
                       (void * const)NULL /* parameter */,
                       (UBaseType_t)10 /* priority */,
                       (TaskHandle_t * const)NULL /* task handle */,
                       (const BaseType_t)APP_CPU_NUM /* core */
                       );
  
  xTaskCreateUniversal(task1, "task1",
                       (const uint32_t)0x2000 /* stack size */,
                       (void * const)NULL /* parameter */,
                       (UBaseType_t)1 /* priority */,
                       (TaskHandle_t * const)NULL /* task handle */,
                       (const BaseType_t)APP_CPU_NUM /* core */
                       );
}

/**
 *
 */
void loop() {
  delay(100);
}
