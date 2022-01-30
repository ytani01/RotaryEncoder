/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_NETMGR_TASK_H_
#define _ESP32_NETMGR_TASK_H_

#include "Esp32Task.h"
#include "Esp32NetMgr.h"

/**
 *
 */
class Esp32NetMgrTask: public Esp32Task {
 public:
  Esp32NetMgr *netMgr = NULL;

  Esp32NetMgrTask(String name, String ap_ssid_hdr,
                  unsigned long wifi_try_count=Esp32NetMgr::DEF_TRY_COUNT_MAX);
  String ap_ssid_hdr;
  unsigned long wifi_try_count;

 protected:
  virtual void setup();
  virtual void loop();
}; // class Esp32NetMgrTask
#endif // _ESP32_NETMGR_TASK_H_
