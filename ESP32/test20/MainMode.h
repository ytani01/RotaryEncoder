/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _MAIN_MODE_H_
#define _MAIN_MODE_H_

#include "ModeBase.h"

/**
 *
 */
class MainMode: public ModeBase {
 public:
  MainMode(String name, CommonData_t *common_data);

  virtual void display(Display_t *disp, float fps);

 protected:
  void drawWiFi(Display_t *disp, int x, int y, Esp32NetMgrInfo_t *ni);
  void drawNtp(Display_t *disp, int x, int y,
               Esp32NtpTaskInfo_t *ntp_info,
               Esp32NetMgrInfo_t *netmgr_info);
  void drawDateTime(Display_t *disp, int x, int y, struct tm *ti);
}; // class MainMode

#endif // _MAIN_MODE_H_
