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
void Esp32NetMgrTask::setup() {
  log_d("%s", this->conf.name);
  this->netMgr = new Esp32NetMgr(this->ap_ssid_hdr, this->wifi_try_count);

  for (int i=0; i < 6; i++) {
    this->netmgr_info->mac_addr[i] = this->netMgr->mac_addr[i];
  } // for(i)

  this->netmgr_info->ext_cmd = "";
} // Esp32NetMgrTask::setup()

/**
 *
 */
void Esp32NetMgrTask::loop() {
  Esp32NetMgrMode_t mode = this->netMgr->loop();

  if ( this->netmgr_info->ext_cmd.length() > 0 ) {
    this->netMgr->ext_cmd = this->netmgr_info->ext_cmd;
    this->netmgr_info->ext_cmd = "";
  }
  
  this->netmgr_info->mode = mode;
  this->netmgr_info->ssid = this->netMgr->cur_ssid;
  if ( mode == NETMGR_MODE_AP_LOOP ) {
    this->netmgr_info->ssid = this->netMgr->ap_ssid;
  }
} // Esp32NetMgrTask::loop()
