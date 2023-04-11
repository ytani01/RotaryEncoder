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

/**
 *
 */
bool SetSsidMode::enter(Mode_t prev_mode) {
  // XXX scan ssid
  
} // SetSsidMode::enter()

/**
 *
 */
Mode_t SetSsidMode::reBtn_cb(Esp32ButtonInfo_t *bi) {
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  return MODE_N;
} // SetSsidMode::reBtn_cb()

/**
 *
 */
Mode_t SetSsidMode::re_cb(Esp32RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle == 0 ) {
    return MODE_N;
  }

  return MODE_N;
} // SetSsidMode::re_cb()

/**
 *
 */
void SetSsidMode::display(Display_t *disp, float fps) {


} // SetSsidMode::display()
