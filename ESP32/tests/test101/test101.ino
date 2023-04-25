/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include <Ticker.h>

#include "common.h"
#include "Display.h"

#include "Mode_Main.h"

#include "Task_Foo.h"
#include "Task_NetMgr.h"
#include "Task_Button.h"

#define LOOP_DELAY 1

CommonData_t commonData;

Task_Foo* task_Foo;

// Display
Display_t *Disp;

// Task
std::vector<Task *> task;

// Timer
constexpr TickType_t TIMER1_INTERVAL = 10 * 1000; // tick == ms (?)
Ticker timer1;

// WiFi
Task_NetMgr* task_NetMgr;

// Button
constexpr uint8_t PIN_BTN_RE = 26;
Task_ButtonWatcher* task_Btn_RE;
ButtonInfo_t btnInfo_RE;

constexpr uint8_t PIN_BTN_OB = 39;
Task_ButtonWatcher* task_Btn_OB;
ButtonInfo_t btnInfo_OB;

// Modes
std::vector<Mode *> mode;
Mode_Main* modeMain;

/**
 *
 */
void cb_timer1() {
  static TickType_t prev_tick1 = xTaskGetTickCount();
  TickType_t tick1 = xTaskGetTickCount();
  TickType_t d_tick1 = tick1 - prev_tick1;
  //log_i("%u:%u", tick1, d_tick1);

  //Serial.println("--------");
  Disp->setFont(NULL);
  Disp->setTextColor(WHITE, BLACK);
  Disp->setTextSize(1);
  //Disp->setCursor(5, 10);
  Disp->setTextWrap(true);
  Disp->printf(" %6d", tick1);
  Disp->display();

  prev_tick1 = tick1;
} // cb_timer1()

/**
 *
 */
void cbBtn_RE(ButtonInfo_t *bi) {
  log_i("%s", Button::info2String(bi).c_str());

  /*
  if ( bi->value == Button::ON ) {
    task_delay(2000);
  }
  */
  
  if ( bi->long_pressed && bi->repeat_count == 0 ) {
    log_i("restart_wifi()");
    task_NetMgr->restart_wifi();
    return;
  }

  if ( bi->click_count == 2 ) {
    log_i("restart_wifi(NETMGR_MODE_AP_INIT)");
    task_NetMgr->restart_wifi(NETMGR_MODE_AP_INIT);
    return;
  }

  if ( bi->click_count >= 3 ) {
    log_w("ESP.restart()");

    WiFi.mode(WIFI_OFF);
    task_delay(100);

    ESP.restart();
  }
} // cbBtn_RE()

void cbBtn_OB(ButtonInfo_t *bi) {
  log_i("%s", Button::info2String(bi).c_str());

  task_delay(1000);
} // cbBtn_OB()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(1000);

  log_i("===== start =====");
  log_d("portTICK_PERIOD_MS=%d", portTICK_PERIOD_MS);
  log_i("MAC Addr: %s", get_mac_addr_String().c_str());

  // Display
  Disp = new Display_t(DISPLAY_W, DISPLAY_H);
  Disp->DispBegin(DISPLAY_I2C_ADDR);
  Disp->setRotation(0);
  Disp->clearDisplay();
  Disp->display();

  // Task
  task_Foo = new Task_Foo("task_foo");
  task.push_back(task_Foo);

  // Task: Button
  task_Btn_RE = new Task_ButtonWatcher("btnRE", PIN_BTN_RE, cbBtn_RE);
  task.push_back(task_Btn_RE);

  task_Btn_OB = new Task_ButtonWatcher("btnOB", PIN_BTN_OB, cbBtn_OB);
  task.push_back(task_Btn_OB);

  // Task: WiFi
  task_NetMgr = new Task_NetMgr("task_NetMgr", "foo");
  task.push_back(task_NetMgr);

  // common data
  commonData.netMgr = task_NetMgr->netMgr;
  commonData.disp = Disp;

  // Task start
  for (int i=0; i < task.size(); i++) {
    task[i]->start();
    task_delay(100);
  }

  // Timer
  timer1.attach_ms(TIMER1_INTERVAL, cb_timer1);

  // Modes
  modeMain = new Mode_Main("Main", Disp);
  mode.push_back(modeMain);

  for (int i=0; i < mode.size(); i++) {
    log_i("%d:%s", i, mode[i]->getName().c_str());
    mode[i]->setup();
  }
  Mode::change_mode(modeMain);

  randomSeed(millis());

} // setup()

/**
 *
 */
void loop() {
  static unsigned long prev_ms = millis();
  unsigned long cur_ms = millis();
  unsigned long d_ms = cur_ms - prev_ms;
  //log_i("%u:%u", cur_ms, d_ms);

  Mode::curMode->loop(cur_ms);

  prev_ms = cur_ms;
  task_delay(LOOP_DELAY);
} // loop()
