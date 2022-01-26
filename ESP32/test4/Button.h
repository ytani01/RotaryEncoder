/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 *
 *  ==========================================================================
 *  example1
 *  --------------------------------------------------------------------------
 *  :
 *  #include "Button.h"
 *  :
 *  Button *btn;
 *  :
 *  void btn_intr_hdr() {
 *    if ( btn->get() ) {
 *      app_btn_intr_hdr(btn);
 *    }
 *  }
 *  
 *  void setup() {
 *    btn = new Button(pin, "button_name", btn_intr_hdr);
 *    :
 *  }
 *  
 *  void loop() {
 *    :
 *    if ( btn->get() ) {
 *      app_btn_loop_hdr(btn);
 *    }
 *    :
 *  }
 *  ==========================================================================
 */
#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>

typedef uint8_t		button_event_t;
typedef uint8_t		count_t;

static const unsigned long BTN_NAME_LEN = 16;

typedef struct {
  uint8_t pin;
  char name[BTN_NAME_LEN + 1];
  bool active;
  bool value;
  bool prev_value;
  unsigned long first_press_start;
  unsigned long press_start;
  count_t count;
  count_t click_count;
  bool long_pressed;
  bool repeated;
} ButtonInfo_t;

class Button {
 public:
  static const unsigned long ON                 = LOW;
  static const unsigned long OFF                = HIGH;

  static const unsigned long DEBOUNCE        	=   10;
  static const unsigned long LONG_PRESS_MSEC 	= 1000;
  static const unsigned long REPEAT_MSEC     	=  300;
  static const unsigned long CLICK_MSEC		=  800;

  ButtonInfo_t info;

  Button(uint8_t pin, String name, void (*intr_hdr)(void));

  bool	get();

  void	enable();
  void	disable();
  bool	is_active();

  String get_name();
  bool 	get_value();
  count_t get_count();
  count_t get_click_count();
  bool is_long_pressed();
  bool is_repeated();

  static String info2String(ButtonInfo_t info, bool interrupted=false);
};

#endif
