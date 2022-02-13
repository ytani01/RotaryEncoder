/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_NETMGR_TASK_H_
#define _ESP32_NETMGR_TASK_H_

#include "Esp32Task.h"
#include "Esp32NetMgr.h"

typedef struct {
  Esp32NetMgrMode_t mode;
  uint8_t mac_addr[6];
  String new_ssid;
  String ssid;
  String ap_ssid;
} Esp32NetMgrInfo_t;

/**
 *
 */
class Esp32NetMgrTask: public Esp32Task {
 public:
  Esp32NetMgr *netMgr = NULL;
  String ap_ssid_hdr;
  unsigned long wifi_try_count;

  Esp32NetMgrTask(String name, String ap_ssid_hdr,
                  Esp32NetMgrInfo_t *netmgr_info,
                  unsigned long wifi_try_count=Esp32NetMgr::DEF_TRY_COUNT_MAX);

  void restart_wifi();
  void clear_ssid();

 protected:
  Esp32NetMgrInfo_t *netmgr_info;

  virtual void setup();
  virtual void loop();

}; // class Esp32NetMgrTask
#endif // _ESP32_NETMGR_TASK_H_
