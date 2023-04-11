/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32ButtonTask.h"

static QueueHandle_t Esp32ButtonTask_queBtn; // XXX

/**
 *
 */
Esp32ButtonTask::Esp32ButtonTask(String name, uint8_t pin,
                                 QueueHandle_t out_que)
  : Esp32Task(name + "_task") {

  this->name = name;
  this->pin = pin;
  this->btn = new Esp32Button(this->name, this->pin,
                              this->task2_btn_intr_hdr);
  Esp32ButtonTask_queBtn = out_que;
} // Esp32ButtonTask::Esp32ButtonTask

/**
 *
 */
void Esp32ButtonTask::setup() {
  log_d("%s", this->conf.name);
} // Esp32ButtonTask::setup()

/**
 *
 */
void Esp32ButtonTask::loop() {
  if ( this->btn->get() ) {
    portBASE_TYPE ret = xQueueSend(Esp32ButtonTask_queBtn,
                                   (void *)&(this->btn->info), 10);
    if ( ret == pdPASS ) {
      log_d("que < %s", this->btn->toString().c_str());
    } else {
      log_w("que X< %s: ret=%d", this->btn->toString().c_str(), ret);
    }
  }
} // Esp32ButtonTask::loop()

/** [static]
 *
 */
void IRAM_ATTR Esp32ButtonTask::task2_btn_intr_hdr(void *btn_obj) {
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
  portBASE_TYPE ret = xQueueSendFromISR(Esp32ButtonTask_queBtn,
                                        (void *)&(btn->info),
                                        &xHigherPriorityTaskWoken);
  if ( ret == pdPASS) {
    isr_log_d("que < %s", btn->toString());
  } else {
    isr_log_e("send que failed: %s: ret=%d", btn->toString(), ret);
  }
  
  if ( xHigherPriorityTaskWoken ) {
    portYIELD_FROM_ISR();
  }
} // task2_btn_intr_hdr()
