/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _MODE_BASE_H_
#define _MODE_BASE_H_

#include <esp32-hal-log.h>
#include "common.h"
#include "Esp32Button.h"
#include "Esp32RotaryEncoder.h"
#include "Display.h"

#define _cur_mode this->common_data->cur_mode

/**
 *
 */
class ModeBase {
public:
  ModeBase(String name, CommonData_t *common_data);

  String get_name();

  virtual void setup();
  virtual bool enter(Mode_t prev_mode);
  virtual bool exit();

  virtual Mode_t reBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t obBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t re_cb(Esp32RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp, float fps=0.0);

protected:
  String name;
  CommonData_t *common_data;
  Mode_t prev_mode;
}; // class ModeBase

#endif // _MODE_BASE_H_
