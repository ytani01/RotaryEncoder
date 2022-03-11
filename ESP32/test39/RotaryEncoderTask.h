/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
 *
### Example
```
#include "RotaryEncoder.h"
:
const uint8_t PIN_DT = 32;
const uint8_t PIN_CLK = 33;
const RotaryEncoderAngle_t ANGLE_MAX = 30;
ButtonWatcher *reWatcher = NULL;
:
void callback_func(RotaryEncoderInfo_t *re_info) {
  log_i("%s", RotaryEncoder::info2String(re_info).c_str());
}
:
void setup() {
  :
  reWatcher = new RotaryEncoderWatcher("RE1", PIN_DT, PIN_CLK, ANGLE_MAX,
                                            callback_func);
  reWatcher->start();
  :
}

void loop() {
  // do nothing
}
```
 */
#ifndef _ROTARY_ENCODER_TASK_H_
#define _ROTARY_ENCODER_TASK_H_

#include "Task.h"
#include "RotaryEncoder.h"

/**
 *
 */
class RotaryEncoderTask: public Task {
 public:
  static const UBaseType_t Q_SIZE = 32;
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;
  
  String re_name;
  uint8_t pin_dt;
  uint8_t pin_clk;
  RotaryEncoderAngle_t angle_max;
  pcnt_ctrl_mode_t lctrl_mode;
  RotaryEncoder *re = NULL;

  RotaryEncoderTask(String re_name,
                         uint8_t pin_dt, uint8_t pin_clk,
                         RotaryEncoderAngle_t angle_max,
                         pcnt_ctrl_mode_t lctrl_mode=PCNT_MODE_KEEP,
                         uint32_t stack_size=STACK_SIZE_DEF,
                         UBaseType_t priority=PRIORITY_DEF,
                         UBaseType_t core=CORE_DEF);

  portBASE_TYPE get(RotaryEncoderInfo_t *re_info);

 protected:
  QueueHandle_t _out_que;

  portBASE_TYPE put();

  virtual void setup();
  virtual void loop();
}; // class RotaryEncoderTask

/**
 *
 */
class RotaryEncoderWatcher: public Task {
public:
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;

  RotaryEncoderWatcher(String re_name,
                            uint8_t pin_dt, uint8_t pin_clk,
                            RotaryEncoderAngle_t angle_max,
                            pcnt_ctrl_mode_t lctrl_mode=PCNT_MODE_KEEP,
                            void (*cb)(RotaryEncoderInfo_t *re_info)=NULL,
                            uint32_t stack_size=STACK_SIZE_DEF,
                            UBaseType_t priority=PRIORITY_DEF,
                            UBaseType_t core=CORE_DEF);

  RotaryEncoderInfo_t *get_re_info_src();
  
protected:
  virtual void setup();
  virtual void loop();
  
  String _re_name;
  uint8_t _pin_dt, _pin_clk;
  RotaryEncoderAngle_t _angle_max;
  pcnt_ctrl_mode_t _lctrl_mode;
  RotaryEncoderTask *_re_task;
  void (*_cb)(RotaryEncoderInfo_t *re_info);

  uint32_t _stack_size;
  UBaseType_t _priority;
  UBaseType_t _core;
}; // class RotaryEncoderWatcher
#endif // _ROTARY_ENCODER_TASK_H_
