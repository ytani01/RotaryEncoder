/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _MENU_MODE_H_
#define _MENU_MODE_H_

#include "ModeBase.h"
#include "OledMenu.h"

/**
 *
 */
class MenuMode: public ModeBase {
 public:
  MenuMode(String name, CommonData_t *common_data);

  virtual void setup();
  virtual bool enter(Mode_t prev_mode);
  //virtual bool exit();
  
  virtual Mode_t reBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t obBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t re_cb(Esp32RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp, float fps);

 protected:
  //  Mode_t dst_mode;
  OledMenu *topMenu, *subMenu;
  OledMenu *curMenu;
}; // class MenuMode

#endif // _MENU_MODE_H_
