/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32NetMgrTask.h"

/**
 *
 */
Esp32NetMgrTask::Esp32NetMgrTask(String name, String ap_ssid_hdr,
                                 unsigned long wifi_try_count)
  : Esp32Task(name + "_task") {

  this->ap_ssid_hdr = ap_ssid_hdr;
  this->wifi_try_count = wifi_try_count;
} // Esp32NetMgrTask::Esp32NetMgrTask

/**
 *
 */
void Esp32NetMgrTask::setup() {
  log_d("%s", this->conf.name);
  this->netMgr = new Esp32NetMgr(this->ap_ssid_hdr, this->wifi_try_count);
} // Esp32NetMgrTask::setup()

/**
 *
 */
void Esp32NetMgrTask::loop() {
  static Esp32NetMgrMode_t prev_mode = NETMGR_MODE_NULL;
  Esp32NetMgrMode_t mode = this->netMgr->loop();

  if ( mode != prev_mode ) {
    log_i("mode=%s", this->netMgr->ModeStr[mode]);
    prev_mode = mode;
  }
} // Esp32NetMgrTask::loop()