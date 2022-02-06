/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <Ticker.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include "Esp32NetMgr.h"

#include "Esp32ButtonTask.h"
#include "Esp32RotaryEncoderTask.h"
#include "OledTask.h"
#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"
#include "OledMenu.h"

// OLED
const uint16_t DISP_W = 128;
const uint16_t DISP_H = 64;
const uint16_t CH_W = 6;
const uint16_t CH_H = 8;
const uint16_t FRAME_W = 3;
Adafruit_SSD1306 *disp;
OledTask *oledTask = NULL;
DispData_t dispData;

// Buttons
const uint8_t PIN_BTN_RE = 26;
const String RE_BTN_NAME = "RotaryEncoderBtn";
Esp32ButtonWatcher *reBtnWatcher = NULL;
Esp32ButtonInfo_t reBtnInfo;

const uint8_t PIN_BTN_ONBOARD = 39;
const String ONBOARD_BTN_NAME = "OnBoardBtn";
Esp32ButtonWatcher *obBtnWatcher = NULL;
Esp32ButtonInfo_t obBtnInfo;

// Rotary Encoder
const String RE_NAME = "RotaryEncoder1";
const uint8_t PIN_PULSE_DT = 33;
const uint8_t PIN_PULSE_CLK = 32;
const int16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
Esp32RotaryEncoderWatcher *reWatcher = NULL;
Esp32RotaryEncoderInfo_t reInfo;

// NeoPixel
const uint8_t PIN_NEOPIXEL_ONBOARD = 27;
const uint16_t LEDS_N_ONBOARD = 1;
const uint8_t LED_BRIGHTNESS_ONBOARD = 50;
CRGB leds_onboard[LEDS_N_ONBOARD];

const uint8_t PIN_NEOPIXEL_EXT1 = 18;
const uint16_t LEDS_N_EXT1 = 100;
const uint8_t LED_BRIGHTNESS_EXT1 = 40;
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
const TickType_t TIMER_INTERVAL = 20 * 1000; // tick == ms (?)
Ticker timer1;

// Menu
OledMenu *menuObj;
OledMenu_t foo1;
OledMenuEnt_t exit_menu;
OledMenuEnt_t foo1_1, foo1_2, foo1_3;
OledMenu_t menu2;
OledMenuEnt_t menu2a, menu2b;

/**
 *
 */
void menu_func_exit() {
  dispData.menu_on = false;
  menuObj->cur = foo1;
  menuObj->cur_ent = 0;
  menuObj->top_ent = 0;
} // menu_func_exit()

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
  // menu: foo1
  strcpy(exit_menu.title, "Return to Clock");
  exit_menu.type = OLED_MENU_ENT_TYPE_FUNC;
  exit_menu.dst.func = menu_func_exit;

  strcpy(foo1_1.title, "REBOOT");
  foo1_1.type = OLED_MENU_ENT_TYPE_FUNC;
  foo1_1.dst.func = menu_func_reboot;

  strcpy(foo1_2.title, "Null");
  foo1_2.type = OLED_MENU_ENT_TYPE_FUNC;
  foo1_2.dst.func = NULL;

  strcpy(foo1_3.title, "> menu2");
  foo1_3.type = OLED_MENU_ENT_TYPE_MENU;
  foo1_3.dst.menu = &menu2;


  strcpy(foo1.title, "Top menu");
  foo1.ent[0] = exit_menu;
  foo1.ent[1] = foo1_2;
  foo1.ent[2] = foo1_2;
  foo1.ent[3] = foo1_1;
  foo1.ent[4] = foo1_2;
  foo1.ent[5] = foo1_2;
  foo1.ent[6] = foo1_2;
  foo1.ent[7] = foo1_2;
  foo1.ent[8] = foo1_3;

  // menu: menu2
  strcpy(menu2a.title, "BACK(Top menu)");
  menu2a.type = OLED_MENU_ENT_TYPE_MENU;
  menu2a.dst.menu = &foo1;

  strcpy(menu2b.title, "BBB");
  menu2b.type = OLED_MENU_ENT_TYPE_FUNC;
  menu2b.dst.func = NULL;

  strcpy(menu2.title, "menu2");
  menu2.ent[0] = menu2a;
  menu2.ent[1] = menu2b;
  menu2.ent[2] = exit_menu;
  menu2.ent[3] = foo1_1;
  
  // menu object
  menuObj = new OledMenu(foo1);
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
void reBtn_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  reBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(255, 0, 255);
  }

  OledMenuEnt_t ment = menuObj->cur.ent[menuObj->cur_ent];
  
  if ( btn_info->click_count == 1 ) {
    if ( dispData.menu_on ) {
      if ( ment.type == OLED_MENU_ENT_TYPE_FUNC && ment.dst.func != NULL ) {
        log_i("%s: call func()", ment.title);
        ment.dst.func();
        return;
      }
      if ( ment.type == OLED_MENU_ENT_TYPE_MENU && ment.dst.menu != NULL ) {
        log_i("%s: > menu %s", ment.title, ment.dst.menu->title);
        menuObj->cur = *ment.dst.menu;
        menuObj->cur_ent = 0;
        menuObj->top_ent = 0;
        return;
      }
    } else {
      dispData.menu_on = true;
    }
    return;
  }

  if ( btn_info->click_count >= 4 ) {
    log_w("restart..");
    strcpy(dispData.cmd, "clear");
    delay(500);
    //ESP.restart();
    ESP.deepSleep(500);
    delay(100);
  }
} // reBtn_cb()

/**
 *
 */
void obBtn_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  obBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(255, 0, 255);
  }

  if ( btn_info->long_pressed && btn_info->repeat_count == 0 ) {
    strcpy(dispData.cmd, "clear");
    log_w("restart..");
    delay(500);
    //ESP.restart();
    ESP.deepSleep(500);
    delay(100);
  }
} // obBtn_cb()

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

  if ( re_info->d_angle > 0 ) {
    dispData.menu->cursor_up();
  } else  if ( re_info->d_angle < 0 ) {
    dispData.menu->cursor_down();
  }
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

  init_menu();
  // init dispData
  dispData.ni = &netMgrInfo;
  dispData.ri1 = &reInfo;
  dispData.bi1 = &reBtnInfo;
  dispData.ntp_info = &ntpInfo;
  dispData.menu = menuObj;
  dispData.menu_on = false;

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
                                            PULSE_MAX,
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