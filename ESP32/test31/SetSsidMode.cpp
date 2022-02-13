/**
 * Copyright (c) 2022 Yoichi Tanibayaishi
 */
#include "SetSsidMode.h"

/** constructor
 *
 */
SetSsidMode::SetSsidMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

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
  log_i("common_data->netmgr_info->new_ssid=%s",
        common_data->netmgr_info->new_ssid.c_str());
  // XXX set ssid

  return true;
} // SetSsidMode::enter()

/** virtual
 *
 */
Mode_t SetSsidMode::reBtn_cb(Esp32ButtonInfo_t *bi) {
  if ( bi->click_count > 1 ) {
    return MODE_MAIN;
  }
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  /*
   * click_count == 1
   */
  
  return MODE_N;
} // SetSsidMode::reBtn_cb()

/** virtual
 *
 */
Mode_t SetSsidMode::re_cb(Esp32RotaryEncoderInfo_t *ri) {

  return MODE_N;
} // SetSsidMode::re_cb()

/** virtual
 *
 */
void SetSsidMode::display(Display_t *disp) {
  int x = 0;
  int y = 0;
  disp->setCursor(x, y);
  disp->setTextSize(2);
  disp->setTextColor(WHITE, BLACK);
  disp->printf("Set SSID");
  disp->drawFastHLine(0, DISPLAY_CH_H * 2 - 1, DISPLAY_W, WHITE);

  y += DISPLAY_CH_H * 5 / 2;
  disp->setCursor(x, y);
  disp->setTextSize(1);
  disp->setTextColor(WHITE, BLACK);
  disp->setTextWrap(true);
  disp->printf("[ SSID ]\n");
  disp->printf("%s\n\n", this->common_data->netmgr_info->new_ssid.c_str());

  y += DISPLAY_CH_H * 5 / 2;
  disp->printf("[ Password ]\n");
  
} // SetSsidMode::display()
