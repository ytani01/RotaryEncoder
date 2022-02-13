/**
 * Copyright (c) Yoichi Tanibayashi
 */
#ifndef _SET_SSID_MODE_H_
#define _SET_SSID_MODE_H_

#include "ModeBase.h"
#include "ConfSsid.h"

/**
 *
 */
class SetSsidMode: public ModeBase {
 public:
  ConfSsid *confSsid;

  SetSsidMode(String name, CommonData_t *common_data);

  virtual void setup();
  virtual bool enter(Mode_t prev_mode);
  virtual Mode_t reBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t re_cb(Esp32RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp);
  
 protected:
  
}; // class SetSsidMode

#endif // _SET_SSID_MODE_H_
