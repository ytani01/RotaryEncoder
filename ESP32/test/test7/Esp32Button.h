/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 *
 *  ==========================================================================
 *  example1
 *  --------------------------------------------------------------------------
 *  :
 *  #include "Esp32Button.h"
 *  :
 *  Esp32Button *btn;
 *  :
 *  void IRAM_ATTR btn_intr_hdr(void *arg_btn) {
 *    Esp32Button *btn = static_cast<Esp32Button *>(arg_btn);
 *    static unsigned long __prev_ms = 0;
 *    unsigned long __cur_ms = millis();
 *    if ( __cur_ms - __prev_ms < Esp32Button::DEBOUNCE ) {
 *      return;
 *    }
 *    __prev_ms = __cur_ms;
 *    if ( ! btn->get() ) {
 *      return;
 *    }
 *    log_d("btn->info.name=%s", btn->info.name);
 *    // ここまで定形
 *
 *    app_btn_intr_hdr();
 *
 *  } // btn_intr_hdr()
 *  
 *  void setup() {
 *    btn = new Esp32Button("button_name", pin, btn_intr_hdr);
 *    :
 *  } // setup()
 *  
 *  void loop() {
 *    :
 *    if ( btn->get() ) {
 *      app_btn_loop_hdr(btn);
 *    }
 *    :
 *  } // loop()
 *  ==========================================================================
 */
#ifndef _ESP32_BUTTON_H
#define _ESP32_BUTTON_H
#include <Arduino.h>


static const unsigned long ESP32_BUTTON_NAME_SIZE = 16;

typedef uint8_t	Esp32ButtonCount_t;

/**
 *
 */
typedef struct {
  char name[ESP32_BUTTON_NAME_SIZE + 1];
  uint8_t pin;
  bool active;
  bool value;
  bool prev_value;
  unsigned long first_press_start;
  unsigned long press_start;
  Esp32ButtonCount_t push_count;
  Esp32ButtonCount_t click_count;
  bool long_pressed;
  Esp32ButtonCount_t repeat_count;;
} Esp32ButtonInfo_t;

/**
 *
 */
class Esp32Button {
 public:
  static const unsigned long ON                 = LOW;
  static const unsigned long OFF                = HIGH;

  static const unsigned long DEBOUNCE        	=   50;
  static const unsigned long LONG_PRESS_MSEC 	= 1000;
  static const unsigned long REPEAT_MSEC     	=  300;
  static const unsigned long CLICK_MSEC		=  800;

  Esp32ButtonInfo_t info;

  Esp32Button(String name, uint8_t pin, void (*intr_hdr)(void *btn));

  bool get();

  void enable();
  void disable();
  bool is_active();

  String get_name();
  bool get_value();
  Esp32ButtonCount_t get_push_count();
  Esp32ButtonCount_t get_click_count();
  bool is_long_pressed();
  Esp32ButtonCount_t get_repeat_count();

  static String info2String(Esp32ButtonInfo_t info, bool interrupted=false);
  String toString(bool interrupted=false);
};
#endif // _ESP32_BUTTON_H
