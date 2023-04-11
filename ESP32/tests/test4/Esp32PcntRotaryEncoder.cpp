/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32PcntRotaryEncoder.h"

/**
 *
 */
Esp32PcntRotaryEncoder::Esp32PcntRotaryEncoder(uint8_t pin_dt,
                                               uint8_t pin_clk,
                                               pcnt_unit_t pcnt_unit,
                                               uint16_t count_max,
                                               void (*intr_hdr)(void *),
                                               void *intr_arg) {
  this->pin_dt = pin_dt;
  this->pin_clk = pin_clk;
  this->pcnt_unit = pcnt_unit;
  this->count_max = count_max;
  this->intr_hdr = intr_hdr;
  this->intr_arg = intr_arg;

  pcnt_config_t cnf;
  cnf.pulse_gpio_num = this->pin_dt;
  cnf.ctrl_gpio_num = this->pin_clk;
  cnf.channel = PCNT_CHANNEL_0;
  cnf.unit = this->pcnt_unit;
  cnf.lctrl_mode = PCNT_MODE_KEEP;
  cnf.hctrl_mode = PCNT_MODE_REVERSE;
  cnf.pos_mode = PCNT_COUNT_INC;
  cnf.neg_mode = PCNT_COUNT_DEC;
  cnf.counter_h_lim = count_max;
  cnf.counter_l_lim = -count_max;
  log_d("pcnt_unit=%d", this->pcnt_unit);
  pcnt_unit_config(&cnf);

  this->pause();
  this->clear();
  this->resume();

  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_ZERO);
  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_H_LIM);
  // pcnt_event_enable(this->pcnt_unit, PCNT_EVT_L_LIM);
  
  pcnt_isr_register(this->intr_hdr, (void *)this->intr_arg,
                    (int)0, &(this->isr_handle));
  pcnt_intr_enable(this->pcnt_unit);
}

/**
 *
 */
int16_t Esp32PcntRotaryEncoder::get() {
  int16_t prev_angle = this->angle;
  
  pcnt_get_counter_value(this->pcnt_unit, &(this->angle));

  this->d_angle = this->angle - prev_angle;
  if ( d_angle < - this->count_max * 50 / 100 ) {
    d_angle += this->count_max;
  }
  if ( d_angle > this->count_max * 50 / 100 ) {
    d_angle -= this->count_max;
  }

  this->angle = (this->angle + this->count_max) % this->count_max;

  return this->d_angle;
}

/**
 *
 */
void Esp32PcntRotaryEncoder::clear() {
  pcnt_counter_clear(this->pcnt_unit);
  this->angle = 0;
  this->d_angle = 0;
}

/**
 *
 */
void Esp32PcntRotaryEncoder::pause() {
  pcnt_counter_pause(this->pcnt_unit);
}

/**
 *
 */
void Esp32PcntRotaryEncoder::resume() {
  pcnt_counter_resume(this->pcnt_unit);
}
