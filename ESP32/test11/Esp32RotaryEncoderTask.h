/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
 *
### Example
```
#include "Esp32RotaryEncoder.h"
:
const uint8_t PIN_DT = 32;
const uint8_t PIN_CLK = 33;
const Esp32RotaryEncoderAngle_t ANGLE_MAX = 30;
Esp32ButtonWatcher *reWatcher = NULL;
:
void callback_func(Esp32RotaryEncoderInfo_t *re_info) {
  log_i("%s", Esp32RotaryEncoder::info2String(re_info).c_str());
}
:
void setup() {
  :
  reWatcher = new Esp32RotaryEncoderWatcher("RE1", PIN_DT, PIN_CLK, ANGLE_MAX,
                                            callback_func);
  reWatcher->start();
  :
}

void loop() {
  // do nothing
}
```
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
  static const UBaseType_t Q_SIZE = 32;
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;
  
  String re_name;
  uint8_t pin_dt;
  uint8_t pin_clk;
  Esp32RotaryEncoderAngle_t angle_max;
  Esp32RotaryEncoder *re = NULL;

  Esp32RotaryEncoderTask(String re_name,
                         uint8_t pin_dt, uint8_t pin_clk,
                         Esp32RotaryEncoderAngle_t angle_max,
                         uint32_t stack_size=STACK_SIZE_DEF,
                         UBaseType_t priority=PRIORITY_DEF,
                         UBaseType_t core=CORE_DEF);

  portBASE_TYPE get(Esp32RotaryEncoderInfo_t *re_info);

 protected:
  QueueHandle_t _out_que;

  portBASE_TYPE put();

  virtual void setup();
  virtual void loop();
}; // class Esp32RotaryEncoderTask

/**
 *
 */
class Esp32RotaryEncoderWatcher: public Esp32Task {
public:
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;

  Esp32RotaryEncoderWatcher(String re_name,
                            uint8_t pin_dt, uint8_t pin_clk,
                            Esp32RotaryEncoderAngle_t angle_max,
                            void (*cb)(Esp32RotaryEncoderInfo_t *re_info)=NULL,
                            uint32_t stack_size=STACK_SIZE_DEF,
                            UBaseType_t priority=PRIORITY_DEF,
                            UBaseType_t core=CORE_DEF);

protected:
  virtual void setup();
  virtual void loop();
  
  String _re_name;
  uint8_t _pin_dt, _pin_clk;
  Esp32RotaryEncoderAngle_t _angle_max;
  Esp32RotaryEncoderTask *_re_task;
  void (*_cb)(Esp32RotaryEncoderInfo_t *re_info);

  uint32_t _stack_size;
  UBaseType_t _priority;
  UBaseType_t _core;
}; // class Esp32RotaryEncoderWatcher
#endif // _ESP32_ROTARY_ENCODER_TASK_H_
