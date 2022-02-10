/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <Ticker.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include "common.h"
#include "Display.h"
#include "MainMode.h"
#include "MenuMode.h"
#include "RestartMode.h"

#include "Esp32ButtonTask.h"
#include "Esp32RotaryEncoderTask.h"
#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"

// Modes
#define _curMode commonData.cur_mode

std::vector<ModeBase *> Mode;
MainMode *mainMode;
MenuMode *menuMode;
RestartMode *restartMode;

// common data
CommonData_t commonData;

// OLED
Display_t *Disp;

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

// BME280
constexpr uint8_t BME280_ADDR = 0x76;
constexpr float TEMP_OFFSET = -1.0;
Esp32Bme280 *Bme;

// Timer
constexpr TickType_t TIMER_INTERVAL = 60 * 1000; // tick == ms (?)
Ticker timer1;

// Menu
OledMenu *menuTop, *menuSub;

/**
 *
 */
bool change_mode(Mode_t mode) {
  if ( ! Mode[_curMode]->exit() ) {
    log_e("%s:exit(): failed", MODE_T_STR[_curMode].c_str());
    return false;
  }

  if ( ! Mode[mode]->enter(_curMode) ) {
    log_e("%s:enter(): failed", MODE_T_STR[mode].c_str());
    return false;
  }

  Mode_t prev_mode = _curMode;
  _curMode = mode;
  log_i("mode: %s ==> %s",
        MODE_T_STR[prev_mode].c_str(),
        MODE_T_STR[_curMode].c_str());
  return true;
} // change_mode()

/** XXX NetMgrにトリガーをかける必要がある
 *
 */
void menuFunc_wifiRestart() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  menuFunc_exitmenu();
} // menuFunc_wifiRestart()

/**
 *
 */
void menuFunc_tempOffset() {
  change_mode(MODE_SET_TEMP_OFFSET);
} // menuFunc_tempOffset()

/**
 *
 */
void menuFunc_exitmenu() {
  OledMenu_curMenu = menuTop;
  change_mode(MODE_MAIN);
} // menuFunc_exitmenu()

/**
 *
 */
void menuFunc_reboot() {
  log_w("restart..");
  commonData.msg = "clear";
  delay(500);

  //ESP.restart();
  ESP.deepSleep(500);
  delay(100);
} // menuFunc_reboot()

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
  
  commonData.msg = " Reboot.. ";
  delay(1000);
  
  //ESP.restart();
  ESP.deepSleep(100);
  delay(100);
} // do_restart()

/**
 *
 */
void menuRe_cb(Esp32RotaryEncoderInfo_t *re_info) {
  if ( re_info->d_angle > 0 ) {
    OledMenu_curMenu->cursor_up();
  } else  if ( re_info->d_angle < 0 ) {
    OledMenu_curMenu->cursor_down();
  }
} // menuRe_cb()

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
void reBtn_cb(Esp32ButtonInfo_t *btn_info) {
  log_i("%s", Esp32Button::info2String(btn_info).c_str());
  reBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(0, 255, 255);
  } else {
    ch_hsv(128, 255, 255);
  }

  if ( btn_info->click_count >= 4 ) {
    do_restart();
    return;
  }

#if 0 // XXX
  if ( _curMode == MODE_MAIN && btn_info->click_count > 0 ) {
    change_mode(MODE_MENU);
    return;
  }
#endif 

  Mode_t dst_mode = Mode[_curMode]->reBtn_cb(btn_info);
  if ( dst_mode != MODE_N && dst_mode != _curMode ) {
    change_mode(dst_mode);
  }
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

  if ( btn_info->long_pressed && btn_info->repeat_count == 0 ) {
    do_restart();
    return;
  }

  Mode_t dst_mode = Mode[_curMode]->obBtn_cb(btn_info);
  if ( dst_mode != MODE_N && dst_mode != _curMode ) {
    change_mode(dst_mode);
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

  Mode_t dst_mode = Mode[_curMode]->re_cb(re_info);
  if ( dst_mode != MODE_N && dst_mode != _curMode ) {
    change_mode(dst_mode);
  }
} // re_cb()

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

  // init commonData
  commonData.netmgr_info = &netMgrInfo;
  commonData.ntp_info = &ntpInfo;

  // BME280
  Bme = new Esp32Bme280(BME280_ADDR, TEMP_OFFSET);
  commonData.bme_info = Bme->get();
  log_i("%f,%f,%f,%f",
        commonData.bme_info->temp,
        commonData.bme_info->hum,
        commonData.bme_info->pres,
        commonData.bme_info->thi);

  // init Display
  Disp = new Display_t(DISPLAY_W, DISPLAY_H);
  Disp->DispBegin(0x3C);
  Disp->clearDisplay();
  Disp->display();

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
  unsigned long task_interval = 10;

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

  // init Mode[]
  mainMode = new MainMode("MainMode", &commonData);
  Mode.push_back(mainMode);
  menuMode = new MenuMode("MenuMode", &commonData);
  Mode.push_back(menuMode);
  restartMode = new RestartMode("RestartMode", &commonData);
  Mode.push_back(restartMode);

  for (int i=0; i < Mode.size(); i++) {
    log_i("%d:%s", i, Mode[i]->get_name().c_str());
    Mode[i]->setup();
  }
  change_mode(MODE_MAIN);  
} // setup()

/**
 *
 */
void loop() {
  static unsigned long prev_ms = millis();
  unsigned long cur_ms = millis();
  int d_ms = cur_ms - prev_ms;
  prev_ms = cur_ms;

  float fps = 0.0;
  if ( d_ms != 0 ) {
    fps = 1000.0 / (float)d_ms;
  }

  Disp->clearDisplay();
  
  if ( commonData.msg.length() > 0 ) {
    log_i("msg:\"%s\"", commonData.msg.c_str());

    Disp->fillRect(0, 0, DISPLAY_W, DISPLAY_H, WHITE);
    Disp->setCursor(0, 10);
    Disp->setTextSize(1);
    Disp->setTextColor(BLACK, WHITE);
    Disp->setTextWrap(true);
    Disp->printf("%s", commonData.msg.c_str());
    Disp->display();

    delay(1500);
    commonData.msg = "";
    return;
  }

  Mode[_curMode]->display(Disp, fps);

  Disp->display();
  delay(1);
}
