/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32RotaryEncoderTask.h"

static QueueHandle_t Esp32RotaryEncoderTask_queRe; // XXX

/**
 *
 */
Esp32RotaryEncoderTask::Esp32RotaryEncoderTask(String name,
                                               uint8_t pin_dt, uint8_t pin_clk,
                                               RotaryEncoderAngle_t angle_max,
                                               QueueHandle_t out_que)
  : Esp32Task(name + "_task") {

  this->name = name;
  this->pin_dt = pin_dt;
  this->pin_clk = pin_clk;
  this->angle_max = angle_max;
  this->re = new Esp32RotaryEncoder(this->name,
                                    this->pin_dt, this->pin_clk,
                                    this->angle_max);

  Esp32RotaryEncoderTask_queRe = out_que;
} // Esp32RotaryEncoderTask::Esp32RotaryEncoderTask

/**
 *
 */
void Esp32RotaryEncoderTask::setup() {
  log_d("%s", this->conf.name);
} // Esp32RotaryEncoderTask::setup()

/**
 *
 */
void Esp32RotaryEncoderTask::loop() {
  RotaryEncoderAngle_t d_angle = this->re->get();
  if ( d_angle == 0 ) {
    delay(1);
    return;
  }

  portBASE_TYPE ret = xQueueSend(Esp32RotaryEncoderTask_queRe,
                                 (void *)&(this->re->info), 10);
  if ( ret == pdPASS ) {
    log_d("que < %s", this->re->toString().c_str());
  } else {
    log_w("que X< %s: ret=%d", this->re->toString().c_str(), ret);
  }
} // Esp32RotaryEncoderTask::loop()
