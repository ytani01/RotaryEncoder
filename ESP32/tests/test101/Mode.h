/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#ifndef _MODE_H_
#define _MODE_H_

#include "common.h"
#include "Button.h"
#include "Display.h"

//#define _cur_mode this->common_data->cur_mode

/**
 *
 */
class Mode {

public:
  static Mode* curMode;
  static Mode* prevMode;

  static Mode* change_mode(Mode* mode);
  
  Mode(String name, Display_t* disp);

  virtual void setup();
  virtual void loop(unsigned long cur_ms);

  virtual void enter();
  virtual void exit();

  //virtual void display();

  /*
  virtual Mode_t reBtn_cb(ButtonInfo_t *bi);
  virtual Mode_t obBtn_cb(ButtonInfo_t *bi);
  virtual Mode_t re_cb(RotaryEncoderInfo_t *ri);
  */

  String getName();

protected:
  String _name;
  Display_t* _disp;
}; // class Mode

#endif // _MODE_H_
