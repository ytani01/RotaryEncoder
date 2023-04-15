/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include "Mode.h"

Mode* Mode::curMode = NULL;
Mode* Mode::prevMode = NULL;

/** constructor
 *
 */
Mode::Mode(String name, Display_t* disp) {
  this->_name = name;
  this->_disp = disp;
} // Mode::Mode()

/** static 
 *
 */
Mode* Mode::change_mode(Mode *mode) {
  // 実際にモードを変える前に、変更前のモードのexit()を実行
  if ( Mode::curMode ) {
    Mode::curMode->exit();
  }

  // change mode
  Mode::prevMode = Mode::curMode;
  Mode::curMode = mode;

  if ( Mode::curMode ) {
    Mode::curMode->enter();
  }
  
  return(Mode::curMode);
} // Mode::change_mode()

/**
 *
 */
String Mode::getName() {
    return this->_name;
} // Mode::getName()
  
/**
 * 最初の初期化
 */
void Mode::setup() {
  log_i("%s", this->_name.c_str());
} // Mode::setup()

/**
 * モード切替時に毎回実行
 */
void Mode::enter() {
  log_i("%s:start", this->_name.c_str());

  this->_disp->clearDisplay();
  this->_disp->setTextSize(1);
  this->_disp->setTextColor(BLACK, WHITE);
  this->_disp->setCursor(5, 10);
  this->_disp->printf(" %s ", this->_name.c_str());

  this->_disp->display();

} // Mode::enter()

/**
 * モード切替時に毎回実行
 */
void Mode::exit() {
  log_i("%s:end", this->_name.c_str());
} // Mode::exit()

/**
 * 
 */
void Mode::loop(unsigned long cur_ms) {
} // Mode::loop()

/**
 *
 */
/*
void Mode::display() {
  this->_disp->setCursor(0, 10);
  this->_disp->printf("%s", __FILE__);
} // Mode::display()
*/
