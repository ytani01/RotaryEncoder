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
  static constexpr char *CH
  = (char *)"\x04 0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&\'()*+,-./:;<=>?/^_~`\\\x11";

  static constexpr char CH_ENTER = 0x04;
  static constexpr char CH_BS = 0x11;

  int ch_i = 0; // 選択中の文字のインデックス
  int cursor_i = 0; // 入力欄でのカーソルの位置

  ConfSsid *confSsid;
  String ssid;
  String pw;

  SetSsidMode(String name, CommonData_t *common_data);

  virtual void setup();
  virtual bool enter(Mode_t prev_mode);
  virtual Mode_t reBtn_cb(Esp32ButtonInfo_t *bi);
  virtual Mode_t re_cb(Esp32RotaryEncoderInfo_t *ri);
  virtual void display(Display_t *disp);
  
 protected:
  
}; // class SetSsidMode

#endif // _SET_SSID_MODE_H_
