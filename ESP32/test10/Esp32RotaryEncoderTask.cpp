/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32RotaryEncoderTask.h"

static QueueHandle_t esp32ReTaskOutQ; // for static function

/**
 *
 */
Esp32RotaryEncoderTask::Esp32RotaryEncoderTask(String re_name,
                                               uint8_t pin_dt, uint8_t pin_clk,
                                               Esp32RotaryEncoderAngle_t angle_max,
                                               uint32_t stack_size,
                                               UBaseType_t priority,
                                               UBaseType_t core):
  Esp32Task(re_name + "Task", stack_size, priority, core) {

  this->re_name = re_name;
  this->pin_dt = pin_dt;
  this->pin_clk = pin_clk;
  this->angle_max = angle_max;

  this->re = new Esp32RotaryEncoder(this->re_name,
                                    this->pin_dt, this->pin_clk,
                                    this->angle_max);

  this->_out_que = xQueueCreate(Esp32RotaryEncoderTask::Q_SIZE,
                                sizeof(Esp32RotaryEncoderInfo_t));
  if ( this->_out_que == NULL ) {
    log_e("create out_que: failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }
  esp32ReTaskOutQ = this->_out_que;
} // Esp32RotaryEncoderTask::Esp32RotaryEncoderTask

/**
 *
 */
void Esp32RotaryEncoderTask::setup() {
  log_d("%s", this->re_name.c_str());
} // Esp32RotaryEncoderTask::setup()

/**
 *
 */
void Esp32RotaryEncoderTask::loop() {
  Esp32RotaryEncoderAngle_t d_angle = this->re->get();
  if ( d_angle == 0 ) {
    delay(1);
    return;
  }

  portBASE_TYPE ret = xQueueSend(esp32ReTaskOutQ,
                                 (void *)&(this->re->info), 1000);
  if ( ret == pdPASS ) {
    log_d("que < %s", this->re->toString().c_str());
  } else {
    log_w("que X< %s: ret=%d", this->re->toString().c_str(), ret);
  }
} // Esp32RotaryEncoderTask::loop()

/**
 *
 */
portBASE_TYPE Esp32RotaryEncoderTask::get(Esp32RotaryEncoderInfo_t *re_info) {
  portBASE_TYPE ret = xQueueReceive(esp32ReTaskOutQ, (void *)re_info, 1000);
  if ( ret == pdPASS ) {
    log_d("que > %s", Esp32RotaryEncoder::info2String(re_info).c_str());
  }
  return ret;
} // Esp32RotaryEncoderTask::get()

/**
 * defulat callback
 */
static void _re_cb(Esp32RotaryEncoderInfo_t *re_info) {
  log_i("%s", Esp32RotaryEncoder::info2String(re_info).c_str());
} // _re_cb()

/**
 *
 */
Esp32RotaryEncoderWatcher::
Esp32RotaryEncoderWatcher(String re_name,
                          uint8_t pin_dt, uint8_t pin_clk,
                          Esp32RotaryEncoderAngle_t angle_max,
                          void (*cb)(Esp32RotaryEncoderInfo_t *re_info),
                          uint32_t stack_size,
                          UBaseType_t priority,
                          UBaseType_t core):
  Esp32Task(re_name + "Watcher", stack_size, priority, core) {

  this->_re_name = re_name;
  this->_pin_dt = pin_dt;
  this->_pin_clk = pin_clk;
  this->_angle_max = angle_max;
  this->_cb = cb;
  this->_stack_size = stack_size;
  this->_priority = priority;
  this->_core = core;

  if ( cb == NULL ) {
    this->_cb = _re_cb;
  }

  this->_re_task=NULL;
} // Esp32RotaryEncoderWatcher::Esp32RotaryEncoderWatcher()

/**
 *
 */
void Esp32RotaryEncoderWatcher::setup() {
  this->_re_task = new Esp32RotaryEncoderTask(this->_re_name,
                                              this->_pin_dt, this->_pin_clk,
                                              this->_angle_max,
                                              this->_stack_size,
                                              this->_priority,
                                              this->_core);
  this->_re_task->start();
} // Esp32RotaryEncoderWatcher::setup()

/**
 *
 */
void Esp32RotaryEncoderWatcher::loop() {
  if ( this->_re_task == NULL ) {
    return;
  }
  
  Esp32RotaryEncoderInfo_t re_info;
  portBASE_TYPE ret = this->_re_task->get(&re_info);
  if ( ret == pdPASS ) {
      (*(this->_cb))(&re_info);
  }
} // Esp32RotaryEncoderWatcher::loop()
