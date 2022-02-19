/**
 * Copyright (c) 2022 Yoichi Tanibayaishi
 */
#include "SetSsidMode.h"

/** constructor
 *
 */
SetSsidMode::SetSsidMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

  log_i("CHARSET=%s", SetSsidMode::CHARSET);
} // SetSsidMode::SetssidMode()

/** virtual
 *
 */
void SetSsidMode::setup() {
  this->confSsid = new ConfSsid();
  this->re_text = new RotaryEncoderText(this,
                                        3, DISPLAY_H - DISPLAY_CH_H, 17,
                                        SetSsidMode::CHARSET,
                                        this->re_text_cb);
} // SetSsidMode::setup()

/** virtual
 *
 */
bool SetSsidMode::enter(Mode_t prev_mode) {
  this->ssid = common_data->netmgr_info->new_ssid;

  confSsid->load();
  this->pw = confSsid->ent[this->ssid.c_str()].c_str();
  log_i("|%s|%s|", this->ssid.c_str(), this->pw.c_str());

  this->re_text->set_text(this->pw);

  this->cursor_i = this->pw.length();
  this->ch_i = 0;
  
  return true;
} // SetSsidMode::enter()

/** virtual
 *
 */
Mode_t SetSsidMode::reBtn_cb(ButtonInfo_t *bi) {
  this->re_text->reBtn_cb(bi);
  return MODE_N;
} // SetSsidMode::reBtn_cb()

/** virtual
 *
 */
Mode_t SetSsidMode::re_cb(RotaryEncoderInfo_t *ri) {
  this->re_text->re_cb(ri);
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
  
  this->re_text->display(disp);
} // SetSsidMode::display()

/** protected static
 *
 */
void SetSsidMode::re_text_cb(char ch, String text, void *mode) {
  log_i("ch=0x%02X, text|%s|", ch, text.c_str());

  SetSsidMode *this_obj = static_cast<SetSsidMode *>(mode);

  this_obj->pw = text;
  this_obj->cursor_i = this_obj->pw.length();

  if ( ch == RotaryEncoderText::CH_ENTER ) {
    this_obj->confSsid->ent[this_obj->ssid.c_str()] = this_obj->pw.c_str();
    this_obj->confSsid->save();
      
    this_obj->common_data->msg = "restart_wifi";
  }
} // SetSsidMode::re_text_cb()
