/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
 * [重要] angleのずれに対する対応
 *   PCNTのカウンター値をそのままangleには使わない!
 *   差分だけを利用する。
 */
#include "RotaryEncoder.h"

/**
 *
 */
RotaryEncoder::RotaryEncoder(String name,
                             uint8_t pin_dt, uint8_t pin_clk,
                             RotaryEncoderAngle_t angle_max,
                             pcnt_ctrl_mode_t lctrl_mode,
                             pcnt_unit_t pcnt_unit,
                             void (*intr_hdr)(void *), void *intr_arg) {
  if ( name.length() > RE_NAME_SIZE ) {
    strcpy(this->info.name, name.substring(0, RE_NAME_SIZE).c_str());
  } else {
    strcpy(this->info.name, name.c_str());
  }
  this->info.pin_dt = pin_dt;
  this->info.pin_clk = pin_clk;
  this->info.angle_max = angle_max;
  this->info.lctrl_mode = lctrl_mode;
  this->pcnt_unit = pcnt_unit;
  this->intr_hdr = intr_hdr;
  this->intr_arg = intr_arg;

  this->info.angle = 0;
  this->info.d_angle = 0;

  pcnt_config_t cnf;
  cnf.pulse_gpio_num = this->info.pin_dt;
  cnf.ctrl_gpio_num = this->info.pin_clk;
  cnf.channel = PCNT_CHANNEL_0;
  cnf.unit = this->pcnt_unit;
  cnf.lctrl_mode = this->info.lctrl_mode;
  cnf.hctrl_mode = PCNT_MODE_REVERSE;
  cnf.pos_mode = PCNT_COUNT_INC;
  cnf.neg_mode = PCNT_COUNT_DEC;
  cnf.counter_h_lim = angle_max;
  cnf.counter_l_lim = -angle_max;
  log_d("pcnt_unit=%d", this->pcnt_unit);
  pcnt_unit_config(&cnf);

  this->pause();
  this->clear();
  this->resume();

  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_ZERO);
  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_H_LIM);
  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_L_LIM);
  
  if ( this->intr_hdr != NULL ) {
    pcnt_isr_register(this->intr_hdr, this->intr_arg,
                      (int)0, &(this->isr_handle));
    pcnt_intr_enable(this->pcnt_unit);
  }
} // RotaryEncoder::RotaryEncoder()

/**
 * [重要] angleのずれに対する対応
 *   PCNTのカウンター値をそのままangleには使わない!
 *   差分だけを利用する。
 */
RotaryEncoderAngle_t RotaryEncoder::get() {
  static RotaryEncoderAngle_t cur_angle;
  RotaryEncoderAngle_t prev_angle = cur_angle;
  
  pcnt_get_counter_value(this->pcnt_unit, &(cur_angle));

  this->info.d_angle = cur_angle - prev_angle;
  this->info.angle += this->info.d_angle;
  
  /*
   * angleが大きく飛んだ場合(XXX 要検討 XXX)
   */
  if ( this->info.d_angle < - this->info.angle_max * 50 / 100 ) {
    this->info.d_angle += this->info.angle_max;
  }
  if ( this->info.d_angle > this->info.angle_max * 50 / 100 ) {
    this->info.d_angle -= this->info.angle_max;
  }

  this->info.angle =
    (this->info.angle + this->info.angle_max) % this->info.angle_max;

  return this->info.d_angle;
} // RotaryEncoder::get()

/**
 *
 */
String RotaryEncoder::get_name() {
  return String(this->info.name);
} // RotaryEncoder::get_name()

/**
 *
 */
void RotaryEncoder::clear() {
  pcnt_counter_clear(this->pcnt_unit);
  this->info.angle = 0;
  this->info.d_angle = 0;
}

/**
 *
 */
void RotaryEncoder::pause() {
  pcnt_counter_pause(this->pcnt_unit);
}

/**
 *
 */
void RotaryEncoder::resume() {
  pcnt_counter_resume(this->pcnt_unit);
} // RotaryEncoder::resume()

/** static
 *
 */
String RotaryEncoder::info2String(RotaryEncoderInfo_t *info) {
  char buf[128];

  sprintf(buf, "RotaryEncoder[%s:%d,%d] %3d %3d/%-3d",
          info->name, info->pin_dt, info->pin_clk,
          info->d_angle, info->angle, info->angle_max);

  return String(buf);
} // RotaryEncoder::info2String()

/**
 *
 */
String RotaryEncoder::toString() {
  return RotaryEncoder::info2String(&(this->info));
} // RotaryEncoder::toString()
