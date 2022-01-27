/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 */
#include "Button.h"

/**
 *
 */
Button::Button(uint8_t pin, String name, void (*intr_hdr)(void))
{
  this->info.pin = pin;
  if ( name.length() > BTN_NAME_LEN ) {
    strcpy(this->info.name, name.substring(0, BTN_NAME_LEN).c_str());
  } else {
    strcpy(this->info.name, name.c_str());
  }
  this->info.value = Button::OFF;
  this->info.prev_value = Button::OFF;
  this->info.press_start = 0;
  this->info.first_press_start = 0;
  this->info.count = 0;
  this->info.click_count = 0;
  this->info.long_pressed = false;
  this->info.repeated = false;

  this->info.active = true;

  pinMode(this->info.pin, INPUT_PULLUP);

  if ( intr_hdr != NULL ) {
    uint8_t intrPin = digitalPinToInterrupt(this->info.pin);
    attachInterrupt(intrPin, intr_hdr, CHANGE);
  }
} // Button::Button()

/**
 * return:
 *	true	changed
 *	false	to be ignored
 */
boolean Button::get()
{
  unsigned long cur_msec = millis();
  boolean 	ret = false;

  if ( ! this->info.active ) {
    return false;
  }

  // Active
  this->info.prev_value = this->info.value;
  this->info.value = digitalRead(this->info.pin);

  this->info.click_count = 0;
  if ( this->info.count > 0 ){
    if ( cur_msec - this->info.first_press_start > CLICK_MSEC ) {
      // click count is detected
      this->info.click_count = this->info.count;
      this->info.count = 0;
      // log_i("[%s] click_count=%d", this->info.name, this->info.click_count);
      ret = true;
    }
  }

  if ( this->info.value == Button::OFF ) {
    // Released button then refresh some flags and do nothing any more
    this->info.press_start = 0;
    this->info.long_pressed = false;
    this->info.repeated = false;

    if ( this->info.prev_value == Button::ON) {
      // Released now !
      // log_i("[%s] released", this->info.name);
      return true;
    }
    // if ( ret ) log_i("[%s] ret=%d", this->info.name, ret);
    return ret;
  }

  // this->info.value == Button::ON
  if ( this->info.prev_value == Button::OFF ) {
    // Pushed now !
    this->info.press_start = cur_msec;
    this->info.count++;
    if ( this->info.count == 1 ) {
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
    this->info.repeated = true;
    this->info.press_start = cur_msec;
    // log_i("[%s] repeat", this->info.name);
    return true;
  }

  // if ( ret ) log_i("[%s] ret=%d", this->info.name, ret);
  return ret;
} // Button::get()

void Button::enable()
{
  this->info.active = true;
}
void Button::disable()
{
  this->info.active = false;
}
boolean Button::is_active()
{
  return this->info.active;
}

String Button::get_name()
{
  return String(this->info.name);
}
boolean Button::get_value()
{
  return this->info.value;
}
count_t Button::get_count()
{
  return this->info.count;
}
count_t Button::get_click_count()
{
  return this->info.click_count;
}
boolean Button::is_long_pressed()
{
  return this->info.long_pressed;
}
boolean Button::is_repeated()
{
  return this->info.repeated;
}

/**
 *
 */
String Button::info2String(ButtonInfo_t info, bool interrupted) {
  char buf[255];
  String intrString = interrupted ? "!" : " ";
  String valueString = info.value ? "H(OFF)" : "L(ON )";
  String longPressedString = info.long_pressed ? "Long" : "----";
  String repeatedString = info.repeated ? "Repeat" : "------";
  
  sprintf(buf, "%sBTN[%s:%d] %s %d %d %s %s",
          intrString.c_str(),
          info.name, info.pin, valueString.c_str(),
          info.count, info.click_count,
          longPressedString.c_str(), repeatedString.c_str());

  return String(buf);
}
