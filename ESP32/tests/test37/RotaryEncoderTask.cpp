/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "RotaryEncoderTask.h"

void IRAM_ATTR intr_hdr(void *p) {
  log_i("AAA");
}

/**
 *
 */
RotaryEncoderTask::RotaryEncoderTask(String re_name,
                                               uint8_t pin_dt, uint8_t pin_clk,
                                               RotaryEncoderAngle_t angle_max,
                                               pcnt_ctrl_mode_t lctrl_mode,
                                               uint32_t stack_size,
                                               UBaseType_t priority,
                                               UBaseType_t core):
  Task(re_name + "Task", stack_size, priority, core) {

  this->re_name = re_name;
  this->pin_dt = pin_dt;
  this->pin_clk = pin_clk;
  this->angle_max = angle_max;
  this->lctrl_mode = lctrl_mode;

  this->re = new RotaryEncoder(this->re_name,
                                    this->pin_dt, this->pin_clk,
                                    this->angle_max,
                                    this->lctrl_mode,
                                    PCNT_UNIT_0,
                                    intr_hdr);

  this->_out_que = xQueueCreate(RotaryEncoderTask::Q_SIZE,
                                sizeof(RotaryEncoderInfo_t));
  if ( this->_out_que == NULL ) {
    log_e("create out_que: failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }
} // RotaryEncoderTask::RotaryEncoderTask

/**
 *
 */
portBASE_TYPE RotaryEncoderTask::put() {
  portBASE_TYPE ret = xQueueSend(this->_out_que,
                                 (void *)&(this->re->info), 1000);
  if ( ret == pdPASS ) {
    log_d("que < %s", this->re->toString().c_str());
  } else {
    log_w("que X< %s: ret=%d", this->re->toString().c_str(), ret);
  }
  return ret;
} // RotaryEncoderTask::put()

/**
 *
 */
portBASE_TYPE RotaryEncoderTask::get(RotaryEncoderInfo_t *re_info) {
  portBASE_TYPE ret = xQueueReceive(this->_out_que, (void *)re_info, 1000);
  if ( ret == pdPASS ) {
    log_d("que > %s", RotaryEncoder::info2String(re_info).c_str());
  }
  return ret;
} // RotaryEncoderTask::get()

/**
 *
 */
void RotaryEncoderTask::setup() {
  log_i("%s", this->re_name.c_str());

  // send initail data
  this->put();
} // RotaryEncoderTask::setup()

/**
 *
 */
void RotaryEncoderTask::loop() {
  RotaryEncoderAngle_t d_angle = this->re->get();
  if ( d_angle == 0 ) {
    delay(1);
    return;
  }

  this->put();
} // RotaryEncoderTask::loop()

/** static
 * defulat callback
 */
static void _re_cb(RotaryEncoderInfo_t *re_info) {
  log_i("%s", RotaryEncoder::info2String(re_info).c_str());
} // _re_cb()

/**
 *
 */
RotaryEncoderWatcher::
RotaryEncoderWatcher(String re_name,
                          uint8_t pin_dt, uint8_t pin_clk,
                          RotaryEncoderAngle_t angle_max,
                          pcnt_ctrl_mode_t lctrl_mode,
                          void (*cb)(RotaryEncoderInfo_t *re_info),
                          uint32_t stack_size, UBaseType_t priority,
                          UBaseType_t core):
  Task(re_name + "Watcher", stack_size, priority, core) {

  this->_re_name = re_name;
  this->_pin_dt = pin_dt;
  this->_pin_clk = pin_clk;
  this->_angle_max = angle_max;
  this->_lctrl_mode = lctrl_mode;
  this->_cb = cb;
  this->_stack_size = stack_size;
  this->_priority = priority;
  this->_core = core;

  if ( cb == NULL ) {
    this->_cb = _re_cb;
  }

  this->_re_task=NULL;
} // RotaryEncoderWatcher::RotaryEncoderWatcher()

/**
 *
 */
RotaryEncoderInfo_t *RotaryEncoderWatcher::get_re_info_src() {
  return &(this->_re_task->re->info);
} // RotaryEncoderWatcher::get_re_info_src()

/**
 *
 */
void RotaryEncoderWatcher::setup() {
  this->_re_task =
    new RotaryEncoderTask(this->_re_name,
                               this->_pin_dt, this->_pin_clk,
                               this->_angle_max,
                               this->_lctrl_mode,
                               this->_stack_size,
                               this->_priority,
                               this->_core);
  this->_re_task->start();
} // RotaryEncoderWatcher::setup()

/**
 *
 */
void RotaryEncoderWatcher::loop() {
  if ( this->_re_task == NULL ) {
    return;
  }
  
  RotaryEncoderInfo_t re_info;
  portBASE_TYPE ret = this->_re_task->get(&re_info);
  if ( ret == pdPASS ) {
    (*(this->_cb))(&re_info); // callback
  }
} // RotaryEncoderWatcher::loop()
