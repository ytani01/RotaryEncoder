/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _TASK_NETMGR_H_
#define _TASK_NETMGR_H_

#include "Task.h"
#include "NetMgr.h"

/**
 *
 */
class Task_NetMgr: public Task {
public:
  NetMgr *netMgr = NULL;
  String ap_ssid_hdr;
  unsigned long wifi_try_count;

  Task_NetMgr(String name, String ap_ssid_hdr,
              unsigned long wifi_try_count=NetMgr::DEF_TRY_COUNT_MAX);

  void restart_wifi(NetMgrMode_t mode=NETMGR_MODE_START);
  void clear_ssid();

protected:
  virtual void setup();
  virtual void loop();

}; // class Task_NetMgr
#endif // _TASK_NETMGR_H_
