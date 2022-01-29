/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_PCNT_ROTARY_ENCODER_H_
#define _ESP32_PCNT_ROTARY_ENCODER_H_

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // requred for PCNT.int_st
#include <driver/pcnt.h>

static const unsigned long ROTARY_ENCODER_NAME_SIZE = 16;

typedef int16_t RotaryEncoderAngle_t;

typedef struct {
  char name[ROTARY_ENCODER_NAME_SIZE + 1];
  uint8_t pin_dt;
  uint8_t pin_clk;
  RotaryEncoderAngle_t angle_max;
  RotaryEncoderAngle_t angle;
  RotaryEncoderAngle_t d_angle;
} RotaryEncoderInfo_t;

class Esp32PcntRotaryEncoder {
 public:
  RotaryEncoderInfo_t info;
  pcnt_unit_t pcnt_unit;
  void (*intr_hdr) (void *arg);
  void *intr_arg = NULL;
  pcnt_isr_handle_t isr_handle;

  Esp32PcntRotaryEncoder(String name,
                         uint8_t pin_dt, uint8_t pin_clk,
                         RotaryEncoderAngle_t angle_max,
                         pcnt_unit_t pcnt_unit=PCNT_UNIT_0,
                         void (*intr_hdr)(void *)=NULL, void *intr_arg=NULL);

  RotaryEncoderAngle_t get();
  String get_name();
  void clear();
  void pause();
  void resume();

  static String info2String(RotaryEncoderInfo_t info);
};
#endif // _ESP32_PCNT_ROTARY_ENCODER_H_
