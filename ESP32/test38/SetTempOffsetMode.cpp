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

/** virtaul
 *
 */
void SetTempOffsetMode::setup() {
  this->conf_temp = new ConfTemp();
  this->conf_temp->load();
  _cd->bme_info->temp_offset = conf_temp->temp_offset;
  log_i("temp_offset=%.1f", _cd->bme_info->temp_offset);
}

/** virtual
 *
 */
Mode_t SetTempOffsetMode::reBtn_cb(ButtonInfo_t *bi) {
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  conf_temp->temp_offset = _cd->bme_info->temp_offset;
  log_i("temp_offset=%.1f", conf_temp->temp_offset);
  conf_temp->save();

  return MODE_MAIN;
} // SetTempOffsetMode::reBtn_cb()

/** virtual
 *
 */
Mode_t SetTempOffsetMode::re_cb(RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle == 0 ) {
    return MODE_N;
  }
  
  float temp_offset = _cd->bme_info->temp_offset;
  if ( ri->d_angle > 0 ) {
    temp_offset += 0.1;
  }
  if ( ri->d_angle < 0 ) {
    temp_offset -= 0.1;
  }
  log_i("temp_offset=%.1f", temp_offset);
  _cd->bme_info->temp_offset = temp_offset;

  return MODE_N;
} // SetTempOffsetMode::re_cb()

/** virtual
 *
 */
void SetTempOffsetMode::display(Display_t *disp) {
  float temp = _cd->bme_info->temp;
  float temp_offset = _cd->bme_info->temp_offset;
  int x, y;

  x = 5;
  y = 10;
  

  disp->setFont(NULL);
  disp->setTextSize(1);

  disp->setTextColor(WHITE, BLACK);
  disp->setCursor(x, y);
  disp->printf("  Temp: %4.1f%cC", temp, (char)247);

  y += 5 + DISPLAY_CH_H * 2;

  disp->setCursor(x, y);
  disp->printf("Offset:       %cC", (char)247);

  x += 45;
  y += DISPLAY_CH_H;
  
  disp->setFont(&FreeSans9pt7b);
  disp->setTextSize(1);

  char buf[10];
  sprintf(buf, "%5.1f", temp_offset);
  
  int16_t x1, y1;
  uint16_t w, h;
  disp->getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  //log_i("(x1,y1,w,h)=(%d,%d,%d,%d)", x1, y1, w, h);

  disp->fillRect(x1 - 2, y1 - 2, w + 4, h + 4, WHITE);

  disp->setTextColor(BLACK, WHITE);
  disp->setCursor(x, y);
  disp->printf("%.1f", temp_offset);

  disp->setFont(NULL);
} // SetTempOffsetMode::display()
