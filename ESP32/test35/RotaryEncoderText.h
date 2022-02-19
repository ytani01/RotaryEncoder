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
    = (char *)"\x04 0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&\'()*+,-./:;<=>?/^_~`\\\x11";
  static constexpr char CH_ENTER = 0x04;
  static constexpr char CH_BS = 0x11;

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
                    void (*cb)(char ch, String text, void *mode_obj)=NULL) {
    this->mode = mode;
    this->x = x;
    this->y = y;
    this->ch_list_len = ch_list_len;
    this->charset = charset;
    this->cb = cb;

    if ( this->ch_list_len % 2 == 0 ) {
      this->ch_list_len--;
    }
    if ( this->ch_list_len < 3 ) {
      this->ch_list_len = 3;
    }
    log_i("ch_list_len=%d", this->ch_list_len);
  };
  
  virtual void set_text(String text) {
    this->out_text = text;
    log_i("out_text|%s|", this->out_text.c_str());
  };

  virtual char reBtn_cb(ButtonInfo_t *bi) {
    char ch = this->charset[this->ch_i];

    if ( bi->click_count > 0 ) {
      if ( ch == RotaryEncoderText::CH_ENTER ) {
        log_i("ch=0x%02d, out_text|%s|", ch, this->out_text);

        this->cb(ch, this->out_text, this->mode);
        return ch;
      }
      return (char)NULL;
    }

    if ( bi->push_count == 0 || bi->value == Button::OFF ) {
      return (char)NULL;
    }

    log_i("ch=%02d", ch);

    switch ( ch ) {
    case RotaryEncoderText::CH_ENTER:
      break;

    case RotaryEncoderText::CH_BS:
      if ( this->out_text.length() == 0 ) {
        break;
      }

      this->out_text.remove(this->out_text.length() - 1);
      log_i("out_text|%s|", this->out_text.c_str());
      break;

    default:
      this->out_text += String(ch);
      log_i("out_text|%s|", this->out_text.c_str());
      break;
    } // switch(ch)

    this->cb(ch, this->out_text, this->mode);
    return ch;
  };

  virtual char re_cb(RotaryEncoderInfo_t *ri) {
    int charset_len = strlen(this->charset);

    if ( ri->d_angle < 0 ) {
      this->ch_i = (this->ch_i + charset_len - 1) % charset_len;
    }
    if ( ri->d_angle > 0 ) {
      this->ch_i = (this->ch_i + charset_len + 1) % charset_len;
    }

    char ch = this->charset[this->ch_i];
    log_i("ch=0x%02d", ch);
    return ch;
  };
  
  virtual void display(Display_t *disp) {
    int charset_len = strlen(this->charset);
    int ch_i1
      = (this->ch_i - (this->ch_list_len / 2) + charset_len) % charset_len;

    int x = this->x;
    int y = this->y;
    
    disp->setFont(NULL);
    disp->setTextSize(1);
    disp->setCursor(x, y);
    for (int i=0; i < this->ch_list_len; i++) {
      int ch_i = (ch_i1 + i + charset_len) % charset_len;
      if ( ch_i == this->ch_i ) {
        disp->drawRect(x - 1, y - 1, DISPLAY_CH_W + 1, DISPLAY_CH_H + 1, WHITE);
        disp->drawChar(x, y, this->charset[ch_i], BLACK, WHITE, 1);
      } else {
        disp->drawChar(x, y, this->charset[ch_i], WHITE, BLACK, 1);
      }

      x += DISPLAY_CH_W + 1;
    } // for(i)
  };

  virtual void display(Display_t *disp, int x, int y, int ch_list_len) {
    this->x = x;
    this->y = y;
    this->ch_list_len = ch_list_len;

    this->display(disp);
  };
  
 protected:
  static void _cb(char ch, String text, void *mode_obj) {
    char buf[16];

    sprintf(buf, " (0x%02X)", ch);
    if ( ch > 0x20 ) {
      buf[0] = ch;
    }
    log_i("%s", buf);
  };
}; // class RotaryEncoderText

#endif // _ROTARY_ENCODER_KEY_
