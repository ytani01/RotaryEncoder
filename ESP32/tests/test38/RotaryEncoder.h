/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // requred for PCNT.int_st
#include <driver/pcnt.h>

static const unsigned long RE_NAME_SIZE = 16;

typedef int16_t RotaryEncoderAngle_t;

typedef struct {
  char name[RE_NAME_SIZE + 1];
  uint8_t pin_dt;
  uint8_t pin_clk;
  RotaryEncoderAngle_t angle_max;
  pcnt_ctrl_mode_t lctrl_mode;
  RotaryEncoderAngle_t angle;
  RotaryEncoderAngle_t d_angle;
} RotaryEncoderInfo_t;

class RotaryEncoder {
 public:
  RotaryEncoderInfo_t info;
  pcnt_unit_t pcnt_unit;
  void (*intr_hdr) (void *arg) = NULL;
  void *intr_arg = NULL;
  pcnt_isr_handle_t isr_handle;

  RotaryEncoder(String name,
                     uint8_t pin_dt, uint8_t pin_clk,
                     RotaryEncoderAngle_t angle_max,
                     pcnt_ctrl_mode_t lctrl_mode=PCNT_MODE_KEEP,
                     pcnt_unit_t pcnt_unit=PCNT_UNIT_0,
                     void (*intr_hdr)(void *)=NULL, void *intr_arg=NULL);

  RotaryEncoderAngle_t get();
  String get_name();
  void clear();
  void pause();
  void resume();

  static String info2String(RotaryEncoderInfo_t *info);
  String toString();
};
#endif // _ROTARY_ENCODER_H_
