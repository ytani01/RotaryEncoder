/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
 * XXX ToDo XXX
 * define info_t angle, d_angle, count_max
 */
#ifndef _ESP32_PCNT_ROTARY_ENCODER_H_
#define _ESP32_PCNT_ROTARY_ENCODER_H_

#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // requred for PCNT.int_st
#include <driver/pcnt.h>

class Esp32PcntRotaryEncoder {
 public:
  uint8_t pin_dt, pin_clk;
  uint16_t count_max;
  int16_t angle = 0;
  int16_t d_angle = 0;
  pcnt_unit_t pcnt_unit;
  void (*intr_hdr) (void *arg);
  void *intr_arg = NULL;
  pcnt_isr_handle_t isr_handle;

  Esp32PcntRotaryEncoder(uint8_t pin_dt, uint8_t pin_clk,
                         pcnt_unit_t pcnt_unit,
                         uint16_t count_max,
                         void (*intr_hdr)(void *), void *intr_arg=NULL);

  int16_t get();
  void clear();
  void pause();
  void resume();
};
#endif // _ESP32_PCNT_ROTARY_ENCODER_H_
