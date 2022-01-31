/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32ButtonTask.h"

static QueueHandle_t outQue = (QueueHandle_t)NULL; // for static function

/**
 *
 */
Esp32ButtonTask::Esp32ButtonTask(String btn_name, uint8_t pin,
                                 uint32_t stack_size,
                                 UBaseType_t priority,
                                 UBaseType_t core):
  Esp32Task(btn_name + "Task", stack_size, priority, core) {

  this->btn_name = btn_name;
  this->pin = pin;

  this->btn = new Esp32Button(this->btn_name, this->pin, this->intr_hdr);

  if ( outQue == NULL ) {
    this->_out_que = xQueueCreate(Esp32ButtonTask::Q_SIZE,
                                  sizeof(Esp32ButtonInfo_t));
    if ( this->_out_que == NULL ) {
      log_e("create out_que: failed .. HALT");
      while (true) { // HALT
        delay(1);
      }
    }
    outQue = this->_out_que;
    log_i("new Que: %X", outQue);
  } else {
    this->_out_que = outQue;
    log_i("reuse Que: %X", outQue);
  }
} // Esp32ButtonTask::Esp32ButtonTask

/**
 *
 */
void Esp32ButtonTask::setup() {
  log_d("%s", this->btn_name.c_str());
} // Esp32ButtonTask::setup()

/**
 *
 */
void Esp32ButtonTask::loop() {
  if ( this->btn->get() ) {
    portBASE_TYPE ret = xQueueSend(outQue,
                                   (void *)&(this->btn->info), 10);
    if ( ret == pdPASS ) {
      log_d("que < %s", this->btn->toString().c_str());
    } else {
      log_w("que X< %s: ret=%d", this->btn->toString().c_str(), ret);
    }
  }
  delay(10);
} // Esp32ButtonTask::loop()

/** [static]
 *
 */
void IRAM_ATTR Esp32ButtonTask::intr_hdr(void *btn_obj) {
  // check debounce
  static unsigned long __prev_ms = 0;
  unsigned long __cur_ms = millis();
  if ( __cur_ms - __prev_ms < Esp32Button::DEBOUNCE ) {
    return;
  }
  __prev_ms = __cur_ms;

  // update button status
  Esp32Button *btn = static_cast<Esp32Button *>(btn_obj);
  if ( ! btn->get() ) {
    return;
  }
  isr_log_d("btn->info.name=%s", btn->info.name);

  // send to queue
  static BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  portBASE_TYPE ret = xQueueSendFromISR(outQue,
                                        (void *)&(btn->info),
                                        &xHigherPriorityTaskWoken);
  if ( ret == pdPASS) {
    isr_log_d("que < %s", btn->toString().c_str());
  } else {
    isr_log_e("send que failed: %s: ret=%d", btn->toString().c_str(), ret);
  }
  
  if ( xHigherPriorityTaskWoken ) {
    isr_log_d("portYIELD_FROM_ISR()");
    portYIELD_FROM_ISR();
  }
} // Esp32ButtonTask::intr_hdr()

/**
 *
 */
portBASE_TYPE Esp32ButtonTask::get(Esp32ButtonInfo_t *btn_info) {
  portBASE_TYPE ret = xQueuePeek(outQue, (void *)btn_info, 1000);
  if ( ret == pdPASS ) {
    if ( String(btn_info->name) == this->btn_name ) {
      ret = xQueueReceive(outQue, (void *)btn_info, 0);
      log_d("%s:que > %s", this->btn_name.c_str(),
            Esp32Button::info2String(btn_info).c_str());
    } else {
      log_d("%s:que >X %s", this->btn_name.c_str(), 
            Esp32Button::info2String(btn_info).c_str());
      ret = pdFALSE;
      delay(2);
    }
  }
  return ret;
} // Esp32ButtonTask::get()

/**
 * defulat callback
 */
static void _button_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
} // _button_cb()

/**
 *
 */
Esp32ButtonWatcher::Esp32ButtonWatcher(String btn_name, uint8_t pin,
                                       void (*cb)(Esp32ButtonInfo_t *btn_info),
                                       uint32_t stack_size,
                                       UBaseType_t priority,
                                       UBaseType_t core):
  Esp32Task(btn_name + "Watcher", stack_size, priority, core) {

  this->_btn_name = btn_name;
  this->_pin = pin;
  this->_cb = cb;
  this->_stack_size = stack_size;
  this->_priority = priority;
  this->_core = core;

  if ( cb == NULL ) {
    this->_cb = _button_cb;
  }

  this->_btn_task=NULL;
} // Esp32ButtonWatcher::Esp32ButtonWatcher()

/**
 *
 */
void Esp32ButtonWatcher::setup() {
  this->_btn_task = new Esp32ButtonTask(this->_btn_name, this->_pin,
                                        this->_stack_size,
                                        this->_priority,
                                        this->_core);
  this->_btn_task->start();
} // Esp32ButtonWatcher::setup()

/**
 *
 */
void Esp32ButtonWatcher::loop() {
  if ( this->_btn_task == NULL ) {
    return;
  }
  
  Esp32ButtonInfo_t btn_info;
  portBASE_TYPE ret = this->_btn_task->get(&btn_info);
  if ( ret == pdPASS ) {
      (*(this->_cb))(&btn_info);
  }
} // Esp32ButtonWatcher::loop()
