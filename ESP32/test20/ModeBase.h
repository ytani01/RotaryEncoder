/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _MODE_BASE_H_
#define _MODE_BASE_H_

#include <esp32-hal-log.h>
#include "common.h"
#include "Esp32Button.h"
#include "Esp32RotaryEncoder.h"
#include "Display.h"

/**
 *
 */
class ModeBase {
public:
  String name;
  CommonData_t *common_data;

  ModeBase(String name, CommonData_t *common_data) {
    this->name = name;
    this->common_data = common_data;
  };

  String get_name() {
    return this->name;
  };
  
  /**
   * 最初の初期化
   */
  virtual void setup() {
    log_i("");
  };

  /**
   * モード切替時に毎回実行
   */
  virtual bool resume() {
    log_i("");
    return true;
  };
  
  virtual void reBtn_cb(Esp32ButtonInfo_t *bi) {
    log_i("");
  };
  
  virtual void obBtn_cb(Esp32ButtonInfo_t *bi) {
    log_i("");
  };
  
  virtual void re_cb(Esp32RotaryEncoderInfo_t *ri) {
    log_i("");
  };
  
  virtual void display(Display_t *disp, float fps) {
    disp->clearDisplay();
    disp->setCursor(0, 0);
    disp->setTextSize(1);
    disp->setTextColor(BLACK, WHITE);
    disp->printf("%s", __FILE__);
    // disp->display();
  };
}; // class ModeBase

#endif // _MODE_BASE_H_
