/**
 * Copyright (c) Yoichi Tanibayashi
 */
#ifndef _SET_SSID_MODE_H_
#define _SET_SSID_MODE_H_

#include "ModeBase.h"
#include "ConfSsid.h"
#include "RotaryEncoderText.h"

class SetSsidMode;

/**
 *
 */
class SetSsidMode: public ModeBase {
 public:
  static constexpr char *CHARSET
  = (char *)"\x04 0123456789.+-*/\xAE\x04 @ABCDEFGHIJKLMNOPQRSTUVWXYZ\xAE\x04 @abcdefghijklmnopqrstuvwxyz\xAE\x04 !\"#$%&\'()*+,-./:;<=>?/^_~`\\\xAE";

  int cursor_i = 0; // 入力欄でのカーソルの位置
  char cur_ch = ' ';

  ConfSsid *confSsid;
  String ssid;
  String pw;

  RotaryEncoderText *re_text;

  SetSsidMode(String name, CommonData_t *common_data);

  virtual void setup();
  virtual bool enter(Mode_t prev_mode);
  virtual Mode_t reBtn_cb(ButtonInfo_t *bi);
  virtual Mode_t re_cb(RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp);
  
 protected:
  static void re_text_cb(char ch, String text, void *mode);
}; // class SetSsidMode

#endif // _SET_SSID_MODE_H_
