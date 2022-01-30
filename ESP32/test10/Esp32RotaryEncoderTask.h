/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_ROTARY_ENCODER_TASK_H_
#define _ESP32_ROTARY_ENCODER_TASK_H_

#include "Esp32Task.h"
#include "Esp32RotaryEncoder.h"

/**
 *
 */
class Esp32RotaryEncoderTask: public Esp32Task {
 public:
  String name = "[NO_NAME]";
  uint8_t pin_dt;
  uint8_t pin_clk;
  RotaryEncoderAngle_t angle_max;
  Esp32RotaryEncoder *re = NULL;

  Esp32RotaryEncoderTask(String name, uint8_t pin_dt, uint8_t pin_clk,
                         RotaryEncoderAngle_t angle_max,
                         QueueHandle_t out_que);

 protected:
  virtual void setup();
  virtual void loop();

  static void task2_re_intr_hdr(void *re_obj);
}; // class Esp32RotaryEncoderTask
#endif // _ESP32_ROTARY_ENCODER_TASK_H_
