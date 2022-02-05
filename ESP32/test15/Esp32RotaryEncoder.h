/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_ROTARY_ENCODER_H_
#define _ESP32_ROTARY_ENCODER_H_

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <soc/pcnt_struct.h> // requred for PCNT.int_st
#include <driver/pcnt.h>

static const unsigned long ESP32_RE_NAME_SIZE = 16;

typedef int16_t Esp32RotaryEncoderAngle_t;

typedef struct {
  char name[ESP32_RE_NAME_SIZE + 1];
  uint8_t pin_dt;
  uint8_t pin_clk;
  Esp32RotaryEncoderAngle_t angle_max;
  Esp32RotaryEncoderAngle_t angle;
  Esp32RotaryEncoderAngle_t d_angle;
} Esp32RotaryEncoderInfo_t;

class Esp32RotaryEncoder {
 public:
  Esp32RotaryEncoderInfo_t info;
  pcnt_unit_t pcnt_unit;
  void (*intr_hdr) (void *arg) = NULL;
  void *intr_arg = NULL;
  pcnt_isr_handle_t isr_handle;

  Esp32RotaryEncoder(String name,
                         uint8_t pin_dt, uint8_t pin_clk,
                         Esp32RotaryEncoderAngle_t angle_max,
                         pcnt_unit_t pcnt_unit=PCNT_UNIT_0,
                         void (*intr_hdr)(void *)=NULL, void *intr_arg=NULL);

  Esp32RotaryEncoderAngle_t get();
  String get_name();
  void clear();
  void pause();
  void resume();

  static String info2String(Esp32RotaryEncoderInfo_t *info);
  String toString();
};
#endif // _ESP32_ROTARY_ENCODER_H_
