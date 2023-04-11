/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_BUTTON_TASK_H_
#define _ESP32_BUTTON_TASK_H_

#include "Esp32Task.h"
#include "Esp32Button.h"

/**
 *
 */
class Esp32ButtonTask: public Esp32Task {
 public:
  String name = "[NO_NAME]";
  uint8_t pin;
  Esp32Button *btn = NULL;

  Esp32ButtonTask(String name, uint8_t pin, QueueHandle_t out_que);

 protected:
  virtual void setup();
  virtual void loop();

  static void task2_btn_intr_hdr(void *btn_obj);
}; // class Esp32ButtonTask
#endif // _ESP32_BUTTON_TASK_H_
