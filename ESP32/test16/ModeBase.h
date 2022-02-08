/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32Button.h"
#include "Esp32RotaryEncoder.h"
#include "Display.h"

/**
 *
 */
class ModeBase {
  ModeBase() {
  };

  virtual void init() {
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

