/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
### Example1 (recommeded): Esp32ButtonWatcher ###
```
#include "Esp32ButtonTask.h"
:
Esp32ButtonWatcher *btnWatcher=NULL;
:
void cb(Esp32ButtonInfo *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  :
}
:
btnWatcher = new Esp32ButtonWatcher("Button name", pin, cb);
btnWatcher.start();
:
```

### Example2: Esp32ButtonTask ###
```
#include "Esp32ButtonTask.h"
:
Esp32NtpTask *btnTask = NULL;
:
btnTask = new Esp32ButtonTask("Button name", pin);
btnTask->start();
:
whie (true) {
  Esp32ButtonInfo btn_info;
  if ( btnTask == NULL ) { // other task
    continue;
  }

  portBASE_TYPE ret = btnTask->get(&btn_info);
  if ( ret == pdPASS ) {
    log_i("%s", Esp32Button::info2String(&btn_info).c_str();
    :
  }
}
:
```
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
  static const UBaseType_t Q_SIZE = 16;
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;

  String btn_name;
  uint8_t pin;
  Esp32Button *btn = NULL;

  Esp32ButtonTask(String btn_name, uint8_t pin,
                  uint32_t stack_size=STACK_SIZE_DEF,
                  UBaseType_t priority=PRIORITY_DEF,
                  UBaseType_t core=CORE_DEF);

  portBASE_TYPE get(Esp32ButtonInfo_t *btn_info);

protected:
  virtual void setup();
  virtual void loop();

  QueueHandle_t _out_que;

  static void intr_hdr(void *btn_obj);
}; // class Esp32ButtonTask

/**
 *
 */
class Esp32ButtonWatcher: public Esp32Task {
public:
  static const uint32_t STACK_SIZE_DEF = 4 * 1024;
  static const UBaseType_t PRIORITY_DEF = 0;
  static const UBaseType_t CORE_DEF = APP_CPU_NUM;

  Esp32ButtonWatcher(String btn_name,
                     uint8_t pin,
                     void (*cb)(Esp32ButtonInfo_t *btn_info)=NULL,
                     uint32_t stack_size=STACK_SIZE_DEF,
                     UBaseType_t priority=PRIORITY_DEF,
                     UBaseType_t core=CORE_DEF);

  Esp32ButtonInfo_t *get_btn_info();

protected:
  virtual void setup();
  virtual void loop();

  String _btn_name;
  uint8_t _pin;
  Esp32ButtonTask *_btn_task;
  void (*_cb)(Esp32ButtonInfo_t *btn_info);

  uint32_t _stack_size;
  UBaseType_t _priority;
  UBaseType_t _core;
}; // class Esp32ButtonWatcher
#endif // _ESP32_BUTTON_TASK_H_
