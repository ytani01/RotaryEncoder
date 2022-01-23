#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // required for PCNT.int_st
#include <driver/pcnt.h>
#include "Button.h"

const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const uint16_t PULSE_COUNT_CYCLE = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

const uint8_t PIN_BTN = 26;
const uint8_t intrPinBtn = digitalPinToInterrupt(PIN_BTN);
Button *btn;
bool flag_btn_intr = false;

/**
 *
 */
static void IRAM_ATTR btn_intr_hdr() {
  log_i("%s> ", __FUNCTION__);
  if ( btn->get() ) {
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

pcnt_isr_handle_t isr_handle;
/**
 *
 */
static void IRAM_ATTR re_intr_hdr(void *arg) {
  log_i("%s> int_str=0x%04X\n", __FUNCTION__, PCNT.int_st.val);
}

/**
 *
 */
void re_init(uint8_t pin_dt, uint8_t pin_clk,
             pcnt_unit_t pcnt_unit,
             uint16_t count_cycle,
             void (*intr_hdr)(void *), void * arg) {
  pcnt_config_t pcnt_config1;
  pcnt_config1.pulse_gpio_num = pin_dt;
  pcnt_config1.ctrl_gpio_num  = pin_clk;

  pcnt_config1.channel        = PCNT_CHANNEL_0;
  pcnt_config1.unit           = pcnt_unit;

  pcnt_config1.lctrl_mode     = PCNT_MODE_KEEP;
  pcnt_config1.hctrl_mode     = PCNT_MODE_REVERSE;

  pcnt_config1.pos_mode       = PCNT_COUNT_INC;
  pcnt_config1.neg_mode       = PCNT_COUNT_DEC;

  pcnt_config1.counter_h_lim  = count_cycle;
  pcnt_config1.counter_l_lim  = -count_cycle;

  pcnt_unit_config(&pcnt_config1);

  pcnt_counter_pause(pcnt_unit);
  pcnt_counter_clear(pcnt_unit);
  pcnt_counter_resume(pcnt_unit);

  pcnt_isr_register(intr_hdr, (void *)arg, (int)0, &isr_handle);
  pcnt_intr_enable(pcnt_unit);
}

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(500);
  log_i("%s> START", __FUNCTION__);

  re_init(PIN_PULSE_CLK, PIN_PULSE_DT, PCNT_UNIT, PULSE_COUNT_CYCLE,
          re_intr_hdr, (void *)NULL);
  
  btn = new Button(PIN_BTN, "Btn");
  enableIntr();
}

/**
 *
 */
int16_t re_get(pcnt_unit_t pcnt_unit) {
  int16_t angle;
  pcnt_get_counter_value(pcnt_unit, &angle);
  return angle;
}

/**
 *
 */
void loop() {
  static int16_t prev_angle = 0;
  static bool flag_clr = false;
  int16_t angle = re_get(PCNT_UNIT);
  int16_t d_angle = angle - prev_angle;

  if ( d_angle != 0 ) {
    Serial.printf("angle=%d, d_angle=%d\n", angle, d_angle);
  }

  if ( flag_btn_intr ) {
    if ( flag_clr ) {
      flag_clr = false;
    } else if ( btn->get_value() == Button::OFF ) {
      Serial.printf("angle=%d, d_angle=%d\n", angle, d_angle);
    }
    flag_btn_intr = false;
  }

  if ( btn->get() ) {
    // btn->print();

    if ( btn->is_long_pressed() && (! btn->is_repeated()) ) {
      // btn->print();
      pcnt_counter_clear(PCNT_UNIT);
      angle = 0;
      prev_angle = 0;
      d_angle = 0;
      flag_clr = true;
      Serial.printf("angle=%d, d_angle=%d\n", angle, d_angle);
    }
  }
  
  prev_angle = angle;
  delay(100);
}
