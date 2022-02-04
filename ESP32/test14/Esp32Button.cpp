/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 */
#include "Esp32Button.h"

/**
 *
 */
Esp32Button::Esp32Button(String name, uint8_t pin, void (*intr_hdr)(void *btn)) {
  if ( name.length() > ESP32_BUTTON_NAME_SIZE ) {
    strcpy(this->info.name, name.substring(0, ESP32_BUTTON_NAME_SIZE).c_str());
  } else {
    strcpy(this->info.name, name.c_str());
  }
  this->info.pin = pin;
  this->info.value = Esp32Button::OFF;
  this->info.prev_value = Esp32Button::OFF;
  this->info.press_start = 0;
  this->info.first_press_start = 0;
  this->info.push_count = 0;
  this->info.click_count = 0;
  this->info.long_pressed = false;
  this->info.repeat_count = 0;

  this->info.active = true;

  pinMode(this->info.pin, INPUT_PULLUP);

  if ( intr_hdr != NULL ) {
    uint8_t intrPin = digitalPinToInterrupt(this->info.pin);
    log_i("%s: intrPin=%d", this->info.name, intrPin);
    attachInterruptArg(intrPin, intr_hdr, this, CHANGE);
  }
} // Esp32Button::Esp32Button()

/**
 * return:
 *	true	changed
 *	false	to be ignored
 */
boolean Esp32Button::get() {
  unsigned long cur_msec = millis();
  boolean 	ret = false;

  if ( ! this->info.active ) {
    return false;
  }

  // Active
  this->info.prev_value = this->info.value;
  this->info.value = digitalRead(this->info.pin);
  this->info.click_count = 0;

  if ( this->info.value == Esp32Button::OFF ) {
    if ( this->info.push_count > 0 ){
      if ( cur_msec - this->info.first_press_start > CLICK_MSEC ) {
        // click count is detected
        this->info.click_count = this->info.push_count;
        this->info.push_count = 0;
        // log_i("[%s] click_count=%d", this->info.name, this->info.click_count);
        ret = true;
      }
    }

    // Released button then refresh some flags and do nothing any more
    this->info.press_start = 0;
    this->info.long_pressed = false;
    this->info.repeat_count = 0;

    if ( this->info.prev_value == Esp32Button::ON) {
      // Released now !
      // log_i("[%s] released", this->info.name);
      return true;
    }
    // if ( ret ) log_i("[%s] ret=%d", this->info.name, ret);
    return ret;
  }

  // this->info.value == Esp32Button::ON
  if ( this->info.prev_value == Esp32Button::OFF ) {
    // Pushed now !
    this->info.press_start = cur_msec;
    this->info.push_count++;
    if ( this->info.push_count == 1 ) {
      this->info.first_press_start = cur_msec;
    }
    // log_i("[%s] pushed", this->info.name);
    return true;
  }

  // check long pressed
  if ( ! this->info.long_pressed ) {
    if ( cur_msec - this->info.press_start > LONG_PRESS_MSEC ) {
      this->info.long_pressed = true;
      this->info.press_start = cur_msec;
      // log_i("[%s] long", this->info.name);
      return true;
    } else {
      // if ( ret ) log_i("[%s] ret=%d", this->info.name, ret);
      return ret;
    }
  }

  // long pressed .. check repeat
  if ( cur_msec - this->info.press_start > REPEAT_MSEC ) {
    this->info.repeat_count++;
    this->info.press_start = cur_msec;
    // log_i("[%s] repeat", this->info.name);
    return true;
  }

  // if ( ret ) log_i("[%s] ret=%d", this->info.name, ret);
  return ret;
} // Esp32Button::get()

/**
 *
 */
void Esp32Button::enable() {
  this->info.active = true;
} // Esp32Button::enable()

/**
 *
 */
void Esp32Button::disable() {
  this->info.active = false;
} // Esp32Button::disable()

/**
 *
 */
boolean Esp32Button::is_active() {
  return this->info.active;
} // Esp32Button::is_active()

/**
 *
 */
String Esp32Button::get_name() {
  return String(this->info.name);
} // Esp32Button::get_name()

/**
 *
 */
boolean Esp32Button::get_value() {
  return this->info.value;
} // Esp32Button::get_value()

/**
 *
 */
Esp32ButtonCount_t Esp32Button::get_push_count() {
  return this->info.push_count;
} // Esp32Button::get_push_count()

/**
 *
 */
Esp32ButtonCount_t Esp32Button::get_click_count() {
  return this->info.click_count;
} // Esp32Button::get_click_count()

/**
 *
 */
boolean Esp32Button::is_long_pressed() {
  return this->info.long_pressed;
} // Esp32Button::is_long_pressed()

/**
 *
 */
Esp32ButtonCount_t Esp32Button::get_repeat_count() {
  return this->info.repeat_count;
} // Esp32Button::get_repeat_count()

/** [static]
 *
 */
String Esp32Button::info2String(Esp32ButtonInfo_t *info, bool interrupted) {
  char buf[128];
  String intrString = interrupted ? "!" : " ";
  String valueString = info->value ? "H(OFF)" : "L(ON )";
  String longPressedString = info->long_pressed ? "L" : "-";
  
  sprintf(buf, "%sBTN[%s:%d] %s P:%d C:%d %s R:%d",
          intrString.c_str(),
          info->name, info->pin, valueString.c_str(),
          info->push_count, info->click_count,
          longPressedString.c_str(), info->repeat_count);

  return String(buf);
} // Esp32Button::info2String()

/**
 *
 */
String Esp32Button::toString(bool interrupted) {
  return Esp32Button::info2String(&(this->info), interrupted);
} // Esp32Button::toString()
