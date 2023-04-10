#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // required for PCNT.int_st
#include <driver/pcnt.h>
#include "Button.h"
#include "Esp32PcntRotaryEncoder.h"

const unsigned long LOOP_DELAY = 10; // ms

const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const uint16_t PULSE_COUNT_CYCLE = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
Esp32PcntRotaryEncoder *re;

const unsigned long DEBOUNCE = 50; // ms
const uint8_t PIN_BTN = 26;
const uint8_t intrPinBtn = digitalPinToInterrupt(PIN_BTN);
Button *btn;
bool flag_btn_intr = false;

/**
 *
 */
void IRAM_ATTR re_intr_hdr(void *arg) {
  log_w("%s> int_str=0x%04X, status=0x%04X\n", __FUNCTION__,
        PCNT.int_st.val, PCNT.status_unit[0].val);

  PCNT.int_clr.val = PCNT.int_st.val;
}

/**
 *
 */
void IRAM_ATTR btn_intr_hdr() {
  static unsigned long prev_ms = 0;
  unsigned long cur_ms = millis();
  unsigned long d_ms = cur_ms - prev_ms;

  if ( d_ms < DEBOUNCE ) {
    return;
  }
  prev_ms = cur_ms;
  
  bool val = btn->get_value();
  log_d("%s> d=%d, val=%d", __FUNCTION__, d_ms, val);

  if ( btn->get() ) {
    //log_i("%s> ", __FUNCTION__);
    flag_btn_intr = true;
  }
}

/**
 *
 */
void enableIntr() {
  attachInterrupt(intrPinBtn, btn_intr_hdr, CHANGE);
}

/**
 *
 */
void disableIntr() {
  detachInterrupt(intrPinBtn);
}

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(500);
  log_i("%s> START", __FUNCTION__);

  re = new Esp32PcntRotaryEncoder(PIN_PULSE_CLK, PIN_PULSE_DT,
                                  PCNT_UNIT, PULSE_COUNT_CYCLE,
                                  re_intr_hdr, (void *)NULL);
  
  btn = new Button(PIN_BTN, "Btn");
  enableIntr();
}

/**
 *
 */
void loop() {
  static int16_t prev_angle = re->angle;
  static bool flag_clr = false;

  if ( re->get() != 0 ) {
    Serial.printf("angle=%d, d_angle=%d\n", re->angle, re->d_angle);
  }

  if ( flag_btn_intr ) {
    if ( flag_clr ) {
      flag_clr = false;
    } else if ( btn->get_value() == Button::OFF ) {
      Serial.printf("angle=%d, d_angle=%d\n", re->angle, re->d_angle);
    }
    flag_btn_intr = false;
  }

  if ( btn->get() ) {
    // btn->print();

    if ( btn->is_long_pressed() && (! btn->is_repeated()) ) {
      // btn->print();
      re->clear();
      flag_clr = true;
      Serial.printf("angle=%d, d_angle=%d\n", re->angle, re->d_angle);
    }
  }

  delay(LOOP_DELAY);
}
