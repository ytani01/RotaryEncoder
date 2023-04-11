/**
 * Copyright (c) Yoichi Tanibayashi
 */
#ifndef _SCAN_SSID_MODE_H_
#define _SCAN_SSID_MODE_H_

#include "ModeBase.h"
#include "OledMenu.h"

/**
 *
 */
class ScanSsidMode: public ModeBase {
 public:
  ScanSsidMode(String name, CommonData_t *common_data);

  virtual bool enter(Mode_t prev_mode);
  virtual bool exit();

  virtual Mode_t reBtn_cb(ButtonInfo_t *bi);
  virtual Mode_t re_cb(RotaryEncoderInfo_t *ri);

  virtual void display(Display_t *disp);
  
 protected:
  OledMenu *ssidMenu;
  std::vector<OledMenuEnt *> ment;
}; // class ScanSsidMode

#endif // _SCAN_SSID_MODE_H_
