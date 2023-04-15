/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Task_NetMgr.h"

/**
 *
 */
Task_NetMgr::Task_NetMgr(String name, String ap_ssid_hdr,
                         unsigned long wifi_try_count)
  : Task(name) {

  this->ap_ssid_hdr = ap_ssid_hdr;
  this->wifi_try_count = wifi_try_count;

  this->netMgr = new NetMgr(this->ap_ssid_hdr, this->wifi_try_count);
} // Task_NetMgr::Task_NetMgr

/**
 *
 */
void Task_NetMgr::restart_wifi(NetMgrMode_t mode) {
  log_i("");
  this->netMgr->restart(mode);
} // Task_NetMgr::restart_wifi()

/**
 *
 */
void Task_NetMgr::clear_ssid() {
  log_i("");
  this->netMgr->save_ssid("", "");
} // Task_NetMgr::clear_ssid()

/**
 *
 */
void Task_NetMgr::setup() {
  log_i("%s", this->conf.name);

} // Task_NetMgr::setup()

/**
 *
 */
void Task_NetMgr::loop() {
  static NetMgrMode_t prev_mode = NETMGR_MODE_NULL;
  NetMgrMode_t mode = this->netMgr->loop();

  if ( mode == NETMGR_MODE_WIFI_ON ) {
      if ( prev_mode != mode ) {
        log_d("ip_addr:%s", this->netMgr->ip_addr.toString().c_str());
      }
  }

  prev_mode = mode;
  task_delay(1);
} // Task_NetMgr::loop()
