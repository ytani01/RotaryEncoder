/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _SET_TEMP_OFFSET_MODE_H_
#define _SET_TEMP_OFFSET_MODE_H_

#include "ModeBase.h"

/**
 *
 */
class SetTempOffsetMode: public ModeBase {
 public:
  SetTempOffsetMode(String name, CommonData_t *common_data);

  virtual Mode_t reBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t re_cb(Esp32RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp, float fps);

 protected:
  
};

#endif // _SET_TEMP_OFFSET_MODE_H_
