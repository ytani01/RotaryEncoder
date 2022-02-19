/**
 * Copyright (c) Yoichi Tanibayashi
 */
#ifndef _ROTARY_ENCODER_KEY_
#define _ROTARY_ENCODER_KEY_

#include "ModeBase.h"
#include "RotaryEncoder.h"
#include "Button.h"
#include "Display.h"

/**
 *
 */
class RotaryEncoderText {
 public:
  static constexpr char *DEF_CHARSET
    = (char *)"\x04 0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&\'()*+,-./:;<=>?/^_~`\\\xAE";

  static constexpr char CH_ENTER = 0x04;
  static constexpr char CH_BS = 0xAE;
  static constexpr char CH_UP = 0x18;
  static constexpr char CH_DOWN = 0x19;
  static constexpr char CH_RIGHT = 0x1B;
  static constexpr char CH_LEFT = 0x1A;

  ModeBase *mode;
  int x, y;
  int ch_list_len;
  char *charset;
  void (*cb)(char ch, String text, void *mode_obj);

  int ch_i = 0;
  String out_text = "";
  
  RotaryEncoderText(ModeBase *mode,
                    int x, int y, int ch_list_len,
                    char *charset=DEF_CHARSET,
                    void (*cb)(char ch, String text, void *mode_obj)=NULL);
  
  char get_ch();
  void set_text(String text);

  virtual char reBtn_cb(ButtonInfo_t *bi);
  virtual char re_cb(RotaryEncoderInfo_t *ri);
  
  virtual void display(Display_t *disp);
  virtual void display(Display_t *disp, int x, int y, int ch_list_len);
  
 protected:
  static void _cb(char ch, String text, void *mode_obj);
}; // class RotaryEncoderText

#endif // _ROTARY_ENCODER_KEY_
