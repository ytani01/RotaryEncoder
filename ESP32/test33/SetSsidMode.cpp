/**
 * Copyright (c) 2022 Yoichi Tanibayaishi
 */
#include "SetSsidMode.h"

/** constructor
 *
 */
SetSsidMode::SetSsidMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

  log_i("CH=%s", SetSsidMode::CH);
} // SetSsidMode::SetssidMode()

/** virtual
 *
 */
void SetSsidMode::setup() {
  this->confSsid = new ConfSsid();
  
} // SetSsidMode::setup()

/** virtual
 *
 */
bool SetSsidMode::enter(Mode_t prev_mode) {
  this->ssid = common_data->netmgr_info->new_ssid;

  confSsid->load();
  this->pw = confSsid->ent[this->ssid.c_str()].c_str();
  log_i("|%s|%s|", this->ssid.c_str(), this->pw.c_str());

  this->cursor_i = this->pw.length();
  
  return true;
} // SetSsidMode::enter()

/** virtual
 *
 */
Mode_t SetSsidMode::reBtn_cb(ButtonInfo_t *bi) {
  char ch = SetSsidMode::CH[this->ch_i];
  
  if ( bi->click_count > 0 ) {
    // モードを抜けるときだけ、クリックカウントで判断
    if ( ch == SetSsidMode::CH_ENTER ) {
      this->confSsid->ent[this->ssid.c_str()] = this->pw.c_str();
      this->confSsid->save();
      
      common_data->msg = "restart_wifi";
      return MODE_MAIN;
    }
    return MODE_N;
  }

  if ( bi->push_count == 0 ) {
    return MODE_N;
  }
  if ( bi->value == Button::OFF ) {
    return MODE_N;
  }
  
  /*
   * value:ON, push_count:1, click_count:0, long_pressed:false
   */
  log_i("ch_i=%d(%c) cursor_i=%d", this->ch_i, ch, this->cursor_i);

  switch ( ch ) {
  case SetSsidMode::CH_ENTER:
    break;

  case SetSsidMode::CH_BS:
    if ( this->cursor_i == 0 ) {
      break;
    }
    this->pw.remove(this->pw.length() - 1);
    this->cursor_i--;
    break;

  default:
    this->pw += String(SetSsidMode::CH[this->ch_i]);
    this->cursor_i++;
    break;
  } // switch(ch)
  return MODE_N;
} // SetSsidMode::reBtn_cb()

/** virtual
 *
 */
Mode_t SetSsidMode::re_cb(RotaryEncoderInfo_t *ri) {
  int ch_len = strlen(SetSsidMode::CH);
  if ( ri->d_angle < 0 ) {
    this->ch_i = (this->ch_i - 1 + ch_len) % ch_len;
  }
  if ( ri->d_angle > 0 ) {
    this->ch_i = (this->ch_i + 1 + ch_len) % ch_len;
  }

  return MODE_N;
} // SetSsidMode::re_cb()

/** virtual
 *
 */
void SetSsidMode::display(Display_t *disp) {
  int x = 0;
  int y = 0;
  disp->setFont(&FreeSans9pt7b);
  int h = 12;
  disp->setTextSize(1);
  disp->setCursor(x, y + h);
  disp->setTextColor(WHITE, BLACK);
  disp->printf("Set SSID");

  y += h + 1;
  disp->drawFastHLine(0, y, DISPLAY_W, WHITE);

  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setTextWrap(true);
  disp->setTextColor(WHITE, BLACK);

  // SSID
  y += 2;
  disp->setCursor(x, y);
  disp->printf("SSID\n");
  
  y += DISPLAY_CH_H - 1;
  disp->drawRect(x + 2, y, DISPLAY_W - 10, DISPLAY_CH_H + 3, WHITE);
  disp->drawFastHLine(x + 2, y, 22, BLACK);

  disp->setCursor(x + 2 + 2, y + 2);
  disp->printf("%s", this->ssid.c_str());

  // Password
  y += DISPLAY_CH_H + 4;
  disp->setCursor(x, y);
  disp->printf("Password\n");

  y += DISPLAY_CH_H - 1;
  disp->drawRect(x + 2, y, DISPLAY_W - 10, DISPLAY_CH_H + 3, WHITE);
  disp->drawFastHLine(x + 2, y, 47, BLACK);
  
  disp->setCursor(x + 2 + 2, y + 2);
  if ( this->pw.length() > 0 ) {
    disp->printf("%s", this->pw.c_str());
  }
  int x_ch = x + this->cursor_i * DISPLAY_CH_W + 2 + 2;
  int y_ch = y + 2;
  char ch = this->pw.charAt(this->cursor_i);

  // cursor
  if ( millis() % 1000 >= 500 ) {
    // disp->drawRect(x_ch, y_ch, DISPLAY_CH_W, DISPLAY_CH_H, WHITE);
  } else {
    disp->fillRect(x_ch, y_ch, DISPLAY_CH_W, DISPLAY_CH_H, WHITE);
  }
  if ( ch ) {
    disp->drawChar(x_ch, y_ch, this->pw.charAt(this->cursor_i), BLACK, WHITE, 1);
  }

  /*
   * char list
   */
  int ch_len = strlen(SetSsidMode::CH);
  int ch_n = 8;
  int ch_i0 = (this->ch_i - ch_n + ch_len) % ch_len;

  x = 3;
  y = DISPLAY_H - DISPLAY_CH_H;
  for (int i=0; i < (ch_n * 2 + 1); i++) {
    int ch_i = (ch_i0 + i + ch_len) % ch_len;
    if ( ch_i == this->ch_i ) {
      disp->drawRect(x - 1, y - 1, DISPLAY_CH_W + 1, DISPLAY_CH_H + 1, WHITE);
      disp->drawChar(x, y, SetSsidMode::CH[ch_i], BLACK, WHITE, 1);
    } else {
      disp->drawChar(x, y, SetSsidMode::CH[ch_i], WHITE, BLACK, 1);
    }
    
    x += DISPLAY_CH_W + 1;
  } // for (i)
} // SetSsidMode::display()
