/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include <Ticker.h>

#include "common.h"

#include "Task_Foo.h"
#include "Task_NetMgr.h"
#include "Task_Button.h"

#define LOOP_DELAY 30000

Task_Foo* task_Foo;

// Timer
constexpr TickType_t TIMER1_INTERVAL = 30 * 1000; // tick == ms (?)
Ticker timer1;

// WiFi
Task_NetMgr* task_NetMgr;
NetMgrInfo_t netMgrInfo;

// Button
constexpr uint8_t PIN_BTN_RE = 26;
Task_ButtonWatcher* task_BtnW_RE;
ButtonInfo_t buttonInfo_RotaryEncoder;

/**
 *
 */
void cb_timer1() {
  static TickType_t prev_tick1 = xTaskGetTickCount();
  TickType_t tick1 = xTaskGetTickCount();
  TickType_t d_tick1 = tick1 - prev_tick1;
  log_i("%u:%u", tick1, d_tick1);

  prev_tick1 = tick1;
} // cb_timer1()

/**
 *
 */
void cbBtn_RE(ButtonInfo_t *bi) {
  log_i("%s", Button::info2String(bi).c_str());

  /*
  ButtonInfo_t* btn1_info = task_BtnW_RE->get_bi();
  log_i("%s", Button::info2String(btn1_info).c_str());
  */

  /*
  if ( bi->value == Button::ON ) {
    task_delay(2000);
  }
  */
  
  if ( bi->long_pressed && bi->repeat_count == 0 ) {
    task_NetMgr->restart_wifi();
    return;
  }

  if ( bi->click_count == 2 ) {
    task_NetMgr->restart_wifi(NETMGR_MODE_AP_INIT);
    return;
  }

  if ( bi->click_count >= 3 ) {
    ESP.restart();
  }
} // cbBtn_RE()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  do {
    task_delay(500);
    log_d(".");
  } while (!Serial);

  log_i("===== start =====");
  log_d("portTICK_PERIOD_MS=%d", portTICK_PERIOD_MS);
  log_i("MAC Addr: %s", get_mac_addr_String().c_str());

  task_Foo = new Task_Foo("task_foo");
  task_Foo->start();

  // Button
  task_BtnW_RE = new Task_ButtonWatcher("btnRE", PIN_BTN_RE, cbBtn_RE);
  task_BtnW_RE->start();

  task_delay(100);

  // WiFi
  task_NetMgr = new Task_NetMgr("task_NetMgr", "foo", &netMgrInfo);
  task_NetMgr->start();

  // Timer
  timer1.attach_ms(TIMER1_INTERVAL, cb_timer1);
} // setup()

/**
 *
 */
void loop() {
  static unsigned long prev_ms = millis();
  unsigned long cur_ms = millis();
  unsigned long d_ms = cur_ms - prev_ms;
  //log_i("%u:%u", cur_ms, d_ms);

  // do something ..

  prev_ms = cur_ms;
  task_delay(LOOP_DELAY);
} // loop()
