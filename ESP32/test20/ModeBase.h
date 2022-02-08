/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <esp-hal-log.h>
#include "Esp32Button.h"
#include "Esp32RotaryEncoder.h"
#include "Display.h"

/**
 *
 */
class ModeBase {
  ModeBase() {
  };

  /**
   * 最初の初期化
   */
  virtual void setup() {
    log_i("");
  };

  /**
   * モード切替時に実行
   */
  virtual void resume() {
    log_i("");
  };
  
  virtual void reBtn_cb(Esp32ButtonInfo_t *bi) {
  };
  
  virtual void obBtn_cb(Esp32ButtonInfo_t *bi) {
  };
  
  virtual void re_cb(Esp32RottaryEncoderInfo_t *ri) {
  };
  
  void display(Display_t disp) {
    
  };
}; // class ModeBase

