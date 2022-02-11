/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "ModeBase.h"

/** constructor
 *
 */
ModeBase::ModeBase(String name, CommonData_t *common_data) {
  this->name = name;
  this->common_data = common_data;
} // ModeBase::ModeBase()

/**
 *
 */
String ModeBase::get_name() {
    return this->name;
} // ModeBase::get_name()
  
/**
 * 最初の初期化
 */
void ModeBase::setup() {
  log_i("");
} // ModeBase::setup()

/**
 * モード切替時に毎回実行
 */
bool ModeBase::enter(Mode_t prev_mode) {
  log_i("%s: prev_mode=%s", this->name.c_str(), MODE_T_STR[prev_mode]);
  this->prev_mode = prev_mode;
  return true;
} // ModeBase::resume()

/**
 * モード切替時に毎回実行
 */
bool ModeBase::exit() {
  log_i("%s", this->name.c_str());
  return true;
} // ModeBase::resume()

/**
 *
 */
Mode_t ModeBase::reBtn_cb(Esp32ButtonInfo_t *bi) {
  log_i("");
  return MODE_N;
} // ModeBase::reBtn_cb()

/**
 *
 */
Mode_t ModeBase::obBtn_cb(Esp32ButtonInfo_t *bi) {
  log_i("");
  return MODE_N;
} // ModeBase::obBtn_cb()

/**
 *
 */
Mode_t ModeBase::re_cb(Esp32RotaryEncoderInfo_t *ri) {
  log_i("");
  return MODE_N;
} // ModeBase::re_cb()

/**
 *
 */
void ModeBase::display(Display_t *disp, float fps) {
  disp->clearDisplay();
  disp->setCursor(0, 0);
  disp->setTextSize(1);
  disp->setTextColor(BLACK, WHITE);
  disp->printf("%s", __FILE__);
  // disp->display();
} // ModeBase::display()
