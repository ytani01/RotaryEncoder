/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32NetMgrTask.h"

/**
 *
 */
Esp32NetMgrTask::Esp32NetMgrTask(String name, String ap_ssid_hdr,
                                 Esp32NetMgrInfo_t *netmgr_info,
                                 unsigned long wifi_try_count):
  Esp32Task(name + "_task") {

  this->ap_ssid_hdr = ap_ssid_hdr;
  this->wifi_try_count = wifi_try_count;
  this->netmgr_info = netmgr_info;
} // Esp32NetMgrTask::Esp32NetMgrTask

/**
 *
 */
void Esp32NetMgrTask::restart_wifi() {
  log_i("");
  this->netMgr->restart();
} // Esp32NetMgrTask::restart_wifi()

/**
 *
 */
void Esp32NetMgrTask::clear_ssid() {
  log_i("");
  this->netMgr->save_ssid("", "");
} // Esp32NetMgrTask::clear_ssid()

/**
 *
 */
void Esp32NetMgrTask::setup() {
  log_d("%s", this->conf.name);
  this->netMgr = new Esp32NetMgr(this->ap_ssid_hdr, this->wifi_try_count);

  this->netmgr_info->ap_ssid = this->netMgr->ap_ssid;
  for (int i=0; i < 6; i++) {
    this->netmgr_info->mac_addr[i] = this->netMgr->mac_addr[i];
  } // for(i)
} // Esp32NetMgrTask::setup()

/**
 *
 */
void Esp32NetMgrTask::loop() {
  static Esp32NetMgrMode_t prev_mode = NETMGR_MODE_NULL;
  Esp32NetMgrMode_t mode = this->netMgr->loop();

  this->netmgr_info->mode = mode;
  this->netmgr_info->ssid = this->netMgr->cur_ssid;
  if ( mode == NETMGR_MODE_WIFI_ON ) {
      this->netmgr_info->ip_addr = this->netMgr->ip_addr;
      if ( prev_mode != mode ) {
        log_i("ip_addr:%s", this->netmgr_info->ip_addr.toString().c_str());
      }
  }

  prev_mode = mode;
} // Esp32NetMgrTask::loop()
