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
 *    btn = new Button("button_name", pin, btn_intr_hdr);
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


static const unsigned long BUTTON_NAME_SIZE = 16;

typedef uint8_t	ButtonCount_t;

typedef struct {
  char name[BUTTON_NAME_SIZE + 1];
  uint8_t pin;
  bool active;
  bool value;
  bool prev_value;
  unsigned long first_press_start;
  unsigned long press_start;
  ButtonCount_t count;
  ButtonCount_t click_count;
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

  Button(String name, uint8_t pin, void (*intr_hdr)(void));

  bool	get();

  void	enable();
  void	disable();
  bool	is_active();

  String get_name();
  bool 	get_value();
  ButtonCount_t get_count();
  ButtonCount_t get_click_count();
  bool is_long_pressed();
  bool is_repeated();

  static String info2String(ButtonInfo_t info, bool interrupted=false);
};

#endif
