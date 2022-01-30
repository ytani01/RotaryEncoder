/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
/**
EXAMPLE
```
#include <esp32-hal-log.h>
#include "Esp32Task.h"

class task1: public Esp32Task {
};
```
 */
#ifndef _ESP32_TASK_H_
#define _ESP32_TASK_H_

#include <Arduino.h>
#include <esp_task_wdt.h>

static const unsigned long ESP32_TASK_NAME_SIZE = 64;

/**
 *
 */
typedef struct {
  char name[ESP32_TASK_NAME_SIZE + 1];
  TaskHandle_t handle;
  uint32_t stack_size;
  UBaseType_t priority;
  uint32_t wdt_sec;
  UBaseType_t core;
} Esp32TaskConf_t;

/**
 *
 */
class Esp32Task {
 public:
  static const uint32_t STACK_SIZE_DEF = 8 * 1024; // bytes
  static const UBaseType_t PRIORITY_DEF = 1;
  static const uint32_t WDT_SEC_DEF = 30; // sec

  Esp32TaskConf_t conf;

  Esp32Task(String name="[NO_NAME_TASK]",
            uint32_t stack_size=STACK_SIZE_DEF,
            UBaseType_t priority=PRIORITY_DEF,
            uint32_t wdt_sec=WDT_SEC_DEF,
            UBaseType_t core=APP_CPU_NUM);
  void start();
  bool is_active();

  void __task_main();

 protected:
  bool _active;

  virtual void setup();
  virtual void loop();

  static void call_task_main(void *this_instance);
}; // class Esp32Task
#endif // _ESP32_TASK_H_
