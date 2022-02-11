/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "SetTempOffsetMode.h"

/** constructor
 *
 */
SetTempOffsetMode::SetTempOffsetMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

} // SetTempOffsetMode::SetTempOffsetMode()

/**
 *
 */
Mode_t SetTempOffsetMode::reBtn_cb(Esp32ButtonInfo_t *bi) {
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  float temp_offset = common_data->bme_info->temp_offset;
  log_i("temp_offset=%.1f", temp_offset);

  return MODE_MAIN;
} // SetTempOffsetMode::reBtn_cb()

/**
 *
 */
Mode_t SetTempOffsetMode::re_cb(Esp32RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle == 0 ) {
    return MODE_N;
  }
  
  float temp_offset = common_data->bme_info->temp_offset;
  if ( ri->d_angle > 0 ) {
    temp_offset += 0.1;
  }
  if ( ri->d_angle < 0 ) {
    temp_offset -= 0.1;
  }
  log_i("temp_offset=%.1f", temp_offset);
  common_data->bme_info->temp_offset = temp_offset;

  return MODE_N;
} // SetTempOffsetMode::re_cb()

/**
 *
 */
void SetTempOffsetMode::display(Display_t *disp, float fps) {
  float temp = common_data->bme_info->temp;
  float temp_offset = common_data->bme_info->temp_offset;

  int x, y;

  x = 5;
  y = 10;
  
  disp->setTextWrap(false);

  disp->setCursor(x, y);
  disp->setTextSize(1);

  disp->setTextColor(WHITE, BLACK);
  disp->printf("  Temp: %4.1f%cC", temp, (char)247);

  disp->setCursor(x, y + 5 + DISPLAY_CH_H * 2);
  disp->printf("Offset:         %cC", (char)247);

  disp->setCursor(x + 45, y + 5 + DISPLAY_CH_H);
  disp->setTextSize(2);
  disp->setTextColor(BLACK, WHITE);
  disp->printf("%.1f", temp_offset);

} // SetTempOffsetMode::display()
