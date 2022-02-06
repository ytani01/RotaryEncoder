/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <Ticker.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include "common.h"
#include "Esp32NetMgr.h"

#include "Esp32ButtonTask.h"
#include "Esp32RotaryEncoderTask.h"
#include "OledTask.h"
#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"
#include "OledMenu.h"

// OLED
constexpr uint16_t DISP_W = 128;
constexpr uint16_t DISP_H = 64;
constexpr uint16_t CH_W = 6;
constexpr uint16_t CH_H = 8;
constexpr uint16_t FRAME_W = 3;
Adafruit_SSD1306 *disp;
OledTask *oledTask = NULL;
DispData_t dispData;

// Buttons
constexpr uint8_t PIN_BTN_RE = 26;
const String RE_BTN_NAME = "RotaryEncoderBtn";
Esp32ButtonWatcher *reBtnWatcher = NULL;
Esp32ButtonInfo_t reBtnInfo;

constexpr uint8_t PIN_BTN_ONBOARD = 39;
const String ONBOARD_BTN_NAME = "OnBoardBtn";
Esp32ButtonWatcher *obBtnWatcher = NULL;
Esp32ButtonInfo_t obBtnInfo;

// Rotary Encoder
const String RE_NAME = "RotaryEncoder1";
constexpr uint8_t PIN_PULSE_DT = 33;
constexpr uint8_t PIN_PULSE_CLK = 32;
/*
 * RE_ANGLE_MAX, LCTRL_MODE は、ロータリーエンコーダーのタイプによって変える
 */
//constexpr int16_t RE_ANGLE_MAX = 30;
//constexpr pcnt_ctrl_mode_t LCTRL_MODE = PCNT_MODE_KEEP;
constexpr int16_t RE_ANGLE_MAX = 20;
constexpr pcnt_ctrl_mode_t RE_LCTRL_MODE = PCNT_MODE_DISABLE;

constexpr pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
Esp32RotaryEncoderWatcher *reWatcher = NULL;
Esp32RotaryEncoderInfo_t reInfo;

// NeoPixel
constexpr uint8_t PIN_NEOPIXEL_ONBOARD = 27;
constexpr uint16_t LEDS_N_ONBOARD = 1;
constexpr uint8_t LED_BRIGHTNESS_ONBOARD = 50;
CRGB leds_onboard[LEDS_N_ONBOARD];

constexpr uint8_t PIN_NEOPIXEL_EXT1 = 18;
constexpr uint16_t LEDS_N_EXT1 = 100;
constexpr uint8_t LED_BRIGHTNESS_EXT1 = 40;
CRGB leds_ext1[LEDS_N_EXT1];

// WiFi
const String AP_SSID_HDR = "test";
Esp32NetMgrTask *netMgrTask = NULL;
Esp32NetMgrInfo_t netMgrInfo;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
Esp32NtpTask *ntpTask = NULL;
Esp32NtpTaskInfo_t ntpInfo;

// Timer
constexpr TickType_t TIMER_INTERVAL = 20 * 1000; // tick == ms (?)
Ticker timer1;

// Menu
OledMenu *menuObj;
OledMenu_t menu1;
OledMenuEnt_t mentExitMenu;
OledMenuEnt_t menu1_1, menu1_2, menu1_3;
OledMenu_t menu2;
OledMenuEnt_t menu2a, menu2b;

// Mode
Mode_t Mode, PrevMode;

/**
 *
 */
Mode_t change_mode(Mode_t mode) {
  PrevMode = Mode;
  Mode = mode;
  dispData.mode = Mode;
  log_i("mode: %s ==> %s", MODE_T_STR[PrevMode], MODE_T_STR[mode]);

  switch ( mode ) {
  case MODE_MENU:
    menuObj->cur = menu1;
    menuObj->cur_ent = 0;
    menuObj->disp_top_ent = 0;
    break;
  } // switch(mode)
      
  return mode;
}

/**
 *
 */
void menu_func_exitmenu() {
  change_mode(PrevMode);
} // menu_func_exitmenu()

/**
 *
 */
void menu_func_reboot() {
  log_w("restart..");
  strcpy(dispData.cmd, "clear");
  delay(500);
  //ESP.restart();
  ESP.deepSleep(500);
  delay(100);
} // menu_func_reboot()

/**
 *
 */
void init_menu() {
  // menu: menu1
  strcpy(mentExitMenu.title, "Return to Clock");
  mentExitMenu.type = OLED_MENU_ENT_TYPE_FUNC;
  mentExitMenu.dst.func = menu_func_exitmenu;

  strcpy(menu1_1.title, "REBOOT");
  menu1_1.type = OLED_MENU_ENT_TYPE_FUNC;
  menu1_1.dst.func = menu_func_reboot;

  strcpy(menu1_2.title, "dummy");
  menu1_2.type = OLED_MENU_ENT_TYPE_FUNC;
  menu1_2.dst.func = NULL;

  strcpy(menu1_3.title, "> menu2");
  menu1_3.type = OLED_MENU_ENT_TYPE_MENU;
  menu1_3.dst.menu = &menu2;


  strcpy(menu1.title, "Top menu");
  menu1.ent.push_back(mentExitMenu);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_1);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_3);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);
  menu1.ent.push_back(menu1_2);

  // menu: menu2
  strcpy(menu2a.title, "BACK(Top menu)");
  menu2a.type = OLED_MENU_ENT_TYPE_MENU;
  menu2a.dst.menu = &menu1;

  strcpy(menu2b.title, "BBB");
  menu2b.type = OLED_MENU_ENT_TYPE_FUNC;
  menu2b.dst.func = NULL;

  strcpy(menu2.title, "menu2");
  menu2.ent.push_back(menu2a);
  menu2.ent.push_back(menu2b);
  menu2.ent.push_back(mentExitMenu);
  menu2.ent.push_back(menu1_1);
  
  // menu object
  menuObj = new OledMenu(menu1);
} // init_menu()

/**
 *
 */
void ch_hsv(uint8_t hue, uint8_t sat, uint8_t val) {
  leds_onboard[0] = CHSV(hue, sat, val);

  for (int i=0; i < LEDS_N_EXT1; i++) {
    leds_ext1[i] = CHSV(hue, sat, val);
    hue = (hue - 32) % 255;
  }
  
  FastLED.show();
} // ch_hsv();

/**
 *
 */
void do_restart() {
  log_w("restart..");
  
  strcpy(dispData.cmd, "clear");
  delay(500);
  
  //ESP.restart();
  ESP.deepSleep(100);
  delay(100);
} // do_restart()

/**
 *
 */
void menuCenterBtn_cb(Esp32ButtonInfo_t *btn_info) {
  OledMenuEnt_t ment = menuObj->cur.ent[menuObj->cur_ent];

  if ( btn_info->click_count == 1 ) {
    if ( ment.type == OLED_MENU_ENT_TYPE_FUNC && ment.dst.func != NULL ) {
      log_i("[%s] call func()", ment.title);
      ment.dst.func();
      return;
    }

    if ( ment.type == OLED_MENU_ENT_TYPE_MENU && ment.dst.menu != NULL ) {
      log_i("[%s]=>[%s]", ment.title, ment.dst.menu->title);
      menuObj->cur = *ment.dst.menu;
      menuObj->cur_ent = 0;
      menuObj->disp_top_ent = 0;
      return;
    }
    return;
  }

  if ( btn_info->click_count >= 4 ) {
    do_restart();
    return;
  }
} // menuCenterBtn_cb()

/**
 *
 */
void reBtn_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  reBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(0, 255, 255);
  } else {
    ch_hsv(128, 255, 255);
  }

  switch ( Mode ) {
  case MODE_MAIN:
    if ( btn_info->click_count == 1 ) {
      change_mode(MODE_MENU);
      return;
    }

    if ( btn_info->click_count >= 4 ) {
      do_restart();
    }
    break;
      
  case MODE_MENU:
    menuCenterBtn_cb(btn_info);
    break;

  default:
    log_e("invalid mode: Mode=%d/%d", Mode, MODE_N);
    break;
  } // swtich (Mode)
} // reBtn_cb()

/**
 *
 */
void obBtn_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  obBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(64, 255, 255);
  } else {
    ch_hsv(192, 255, 255);
  }

  switch ( Mode ) {
  case MODE_MAIN:
    if ( btn_info->long_pressed && btn_info->repeat_count == 0 ) {
      do_restart();
    }
    break;
      
  default:
    log_e("invalid mode: Mode=%d/%d", Mode, MODE_N);
    break;
  } // swtich (Mode)
} // obBtn_cb()

/**
 *
 */
void menuRe_cb(Esp32RotaryEncoderInfo_t *re_info) {
  if ( re_info->d_angle > 0 ) {
    menuObj->cursor_up();
  } else  if ( re_info->d_angle < 0 ) {
    menuObj->cursor_down();
  }
} // menuRe_cb()

/**
 *
 */
void re_cb(Esp32RotaryEncoderInfo_t *re_info) {
  log_i("%s", Esp32RotaryEncoder::info2String(re_info).c_str());

  /**
   * XXX
   * RotaryEncoderのボタンを押した瞬間に、
   * d_angle=2*n の入力が検知されることがある !??
   */
  if ( re_info->d_angle % 2 == 0 ) {
    log_w("d_angle=%d: ignored", re_info->d_angle);

    // XXX angleを戻す
    Esp32RotaryEncoderInfo_t *re_info_src = reWatcher->get_re_info();
    re_info_src->angle -= re_info->d_angle;
    return;
  }

  switch ( Mode ) {
  case MODE_MENU:
    menuRe_cb(re_info);
    break;
  } // switch (Mode)

  reInfo = *re_info;
} // re_cb()

/**
 *
 */
void ntp_cb(Esp32NtpTaskInfo_t *ntp_info) {
  log_i("sntp_stat=%d", ntp_info->sntp_stat);
  
  ntpInfo = *ntp_info;
} // ntp_cb()

/**
 * 【注意・重要】
 * コールバック実行中に、次のタイマー時間になると、
 * 次のコールバックが待たされ、あふれるとパニックする。
 */
void timer1_cb() {
  TickType_t tick1 = xTaskGetTickCount();
  log_i("[%s] timer test: start(priority=%d)",
        Esp32NtpTask::get_time_str(), uxTaskPriorityGet(NULL));

  delay(TIMER_INTERVAL / 2);

  TickType_t tick2 = xTaskGetTickCount();
  TickType_t d_tick = tick2 - tick1;
  log_d("%d %d", tick1, tick2);
  log_d("[%s] timer test: end(d_tick=%d)",
        Esp32NtpTask::get_time_str(), d_tick);
} // timer1_cb()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  do {
    delay(100);
  } while (!Serial);  // Serial Init Wait

  Serial.println();
  Serial.println("===== start =====");
  log_i("portTICK_PERIOD_MS=%d", portTICK_PERIOD_MS);

  Mode = MODE_MAIN;

  init_menu();
  // init dispData
  dispData.ni = &netMgrInfo;
  dispData.ri1 = &reInfo;
  dispData.bi1 = &reBtnInfo;
  dispData.ntp_info = &ntpInfo;
  dispData.menu = menuObj;

  // NeoPixel
  FastLED.addLeds<WS2812B, PIN_NEOPIXEL_ONBOARD, GRB>
    (leds_onboard, LEDS_N_ONBOARD).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS_ONBOARD);
  leds_onboard[0] = CRGB(255,255,255);

  FastLED.addLeds<WS2812B, PIN_NEOPIXEL_EXT1, GRB>
    (leds_ext1, LEDS_N_EXT1).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS_EXT1);
  for (int i=0; i < LEDS_N_EXT1; i++) {
    leds_ext1[i] = CRGB(255,255,255);
  }
  FastLED.show();

  // Tasks
  unsigned long task_interval = 50;

  oledTask = new OledTask(&dispData);
  oledTask->start();
  delay(task_interval);

  ntpTask = new Esp32NtpTask((String *)NTP_SVR, &netMgrTask, ntp_cb);
  ntpTask->start();
  delay(task_interval);

  netMgrTask = new Esp32NetMgrTask("NetMgr", AP_SSID_HDR, &netMgrInfo);
  netMgrTask->start();
  delay(task_interval);

  reBtnWatcher = new Esp32ButtonWatcher(RE_BTN_NAME, PIN_BTN_RE,
                                        reBtn_cb);
  reBtnWatcher->start();
  delay(task_interval);

  obBtnWatcher = new Esp32ButtonWatcher(ONBOARD_BTN_NAME, PIN_BTN_ONBOARD,
                                        obBtn_cb);
  obBtnWatcher->start();
  delay(task_interval);

  reWatcher = new Esp32RotaryEncoderWatcher(RE_NAME,
                                            PIN_PULSE_DT, PIN_PULSE_CLK,
                                            RE_ANGLE_MAX, RE_LCTRL_MODE,
                                            re_cb);
  reWatcher->start();
  delay(task_interval);

  // start timer1
  timer1.attach_ms(TIMER_INTERVAL, timer1_cb);
  log_i("start Timer: %.1f sec", TIMER_INTERVAL / 1000.0);
} // setup()

/**
 *
 */
void loop() {
  delay(100);
}
