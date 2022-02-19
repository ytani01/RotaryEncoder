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
#include "SetTempOffsetMode.h"
#include "ScanSsidMode.h"
#include "SetSsidMode.h"

#include "ButtonTask.h"
#include "RotaryEncoderTask.h"
#include "NetMgrTask.h"
#include "NtpTask.h"
#include "MqttTask.h"

#include "ConfFps.h"

ConfFps *confFps;
static bool dispFps = true;

// Modes
#define curMode commonData.cur_mode

std::vector<ModeBase *> Mode;
MainMode *mainMode;
MenuMode *menuMode;
RestartMode *restartMode;
SetTempOffsetMode *setTempOffsetMode;
ScanSsidMode *scanSsidMode;
SetSsidMode *setSsidMode;

// common data
CommonData_t commonData;

// OLED
Display_t *Disp;

// Buttons
constexpr uint8_t PIN_BTN_RE = 26;
const String RE_BTN_NAME = "RotaryEncoderBtn";
ButtonWatcher *reBtnWatcher = NULL;
ButtonInfo_t reBtnInfo;

constexpr uint8_t PIN_BTN_ONBOARD = 39;
const String ONBOARD_BTN_NAME = "OnBoardBtn";
ButtonWatcher *obBtnWatcher = NULL;
ButtonInfo_t obBtnInfo;

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
RotaryEncoderWatcher *reWatcher = NULL;
RotaryEncoderInfo_t reInfo;

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
const String AP_SSID_HDR = "iot";
NetMgrTask *netMgrTask = NULL;
NetMgrInfo_t netMgrInfo;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
NtpTask *ntpTask = NULL;
NtpTaskInfo_t ntpInfo;

// MQTT
const String MQTT_SERVER = "mqtt.ytani.net";
const int MQTT_PORT = 1883;
const String MQTT_TOPIC_ROOT = "esp32"; // topic = MQTT_TOPIC_ROOT + "/" + RES_NAME
const String MQTT_CLIENT_ID = "esp32client";
MqttTask *mqttTask = NULL;

// BME280
constexpr uint8_t BME280_ADDR = 0x76;
constexpr float TEMP_OFFSET = -1.0;
Bme280 *Bme;

// Timer
constexpr TickType_t TIMER_INTERVAL = 10 * 1000; // tick == ms (?)
Ticker timer1;

// Menu
OledMenu *menuTop, *menuSub;

/**
 *
 */
bool change_mode(Mode_t mode) {
  if ( ! Mode[curMode]->exit() ) {
    log_e("%s:exit(): failed", MODE_T_STR[curMode]);
    return false;
  }

  if ( ! Mode[mode]->enter(curMode) ) {
    log_e("%s:enter(): failed", MODE_T_STR[mode]);
  }

  Mode_t prev_mode = curMode;
  curMode = mode;
  log_i("mode: %s ==> %s",
        MODE_T_STR[prev_mode], MODE_T_STR[curMode]);
  return true;
} // change_mode()

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
  delay(500);
} // do_restart()

/**
 *
 */
void ntp_cb(NtpTaskInfo_t *ntp_info) {
  log_d("sntp_stat=%s(%d)",
        SNTP_SYNC_STATUS_STR[ntp_info->sntp_stat], ntp_info->sntp_stat);
  
  ntpInfo = *ntp_info;
} // ntp_cb()

/**
 * 【注意・重要】
 * コールバック実行中に、次のタイマー時間になると、
 * 次のコールバックが待たされ、あふれるとパニックする。
 */
void timer1_cb() {
  TickType_t tick1 = xTaskGetTickCount();
  log_d("[%s] timer test: start(priority=%d)",
        NtpTask::get_time_str(), uxTaskPriorityGet(NULL));

  commonData.bme_info = Bme->get();
  float temp_offset = Bme->get_temp_offset();
  log_d("%s: Bme->get(): %.2f(%.1f), %.1f, %.1f, %.1f",
        NtpTask::get_time_str(),
        commonData.bme_info->temp, temp_offset,
        commonData.bme_info->hum,
        commonData.bme_info->pres,
        commonData.bme_info->thi);

  //  delay(TIMER_INTERVAL / 2);

  TickType_t tick2 = xTaskGetTickCount();
  TickType_t d_tick = tick2 - tick1;
  log_d("%d %d", tick1, tick2);
  log_d("[%s] timer test: end(d_tick=%d)",
        NtpTask::get_time_str(), d_tick);
} // timer1_cb()

/** callback
 *
 */
void reBtn_cb(ButtonInfo_t *btn_info) {
  log_i("%s", Button::info2String(btn_info).c_str());
  reBtnInfo = *btn_info;

  if ( btn_info->push_count > 0 ) {
    ch_hsv(0, 255, 255);
  } else {
    ch_hsv(128, 255, 255);
  }

#if 0
  if ( btn_info->click_count >= 4 ) {
    do_restart();
    return;
  }
#endif

  Mode_t dst_mode = Mode[curMode]->reBtn_cb(btn_info);
  if ( dst_mode != MODE_N && dst_mode != curMode ) {
    change_mode(dst_mode);
  }
} // reBtn_cb()

/** callback
 *
 */
void obBtn_cb(ButtonInfo_t *btn_info) {
  log_i("%s", Button::info2String(btn_info).c_str());
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

  Mode_t dst_mode = Mode[curMode]->obBtn_cb(btn_info);
  if ( dst_mode != MODE_N && dst_mode != curMode ) {
    change_mode(dst_mode);
  }
} // obBtn_cb()

/** callback
 *
 */
void re_cb(RotaryEncoderInfo_t *re_info) {
  log_d("%s", RotaryEncoder::info2String(re_info).c_str());

  /**
   * XXX
   * RotaryEncoderのボタンを押した瞬間に、
   * d_angle=2*n の入力が検知されることがある !??
   */
  if ( re_info->d_angle % 2 == 0 ) {
    log_w("d_angle=%d: ignored", re_info->d_angle);

    // XXX angleを戻す
    RotaryEncoderInfo_t *re_info_src = reWatcher->get_re_info_src();
    re_info_src->angle -= re_info->d_angle;
    return;
  }

  if ( re_info->d_angle == 0 ) {
    log_w("d_angle=%d: ignored", re_info->d_angle);
    return;
  }

  Mode_t dst_mode = Mode[curMode]->re_cb(re_info);
  if ( dst_mode != MODE_N && dst_mode != curMode ) {
    change_mode(dst_mode);
  }
} // re_cb()

/** callback
 *
 */
void menu_cb(String text) {
  log_i("text=%s", text.c_str());

  if ( text == "disp_fps" ) {
    if ( dispFps ) {
      dispFps = false;
    } else {
      dispFps = true;
    }
    log_i("dispFps=%d,%d,%d", dispFps, true, false);
    confFps->disp_fps = dispFps;
    confFps->save();
    change_mode(MODE_MAIN);
    return;
  }

  if ( text == "reboot" ) {
    do_restart();
  }

  if ( text == "clear_ssid" ) {
    netMgrTask->clear_ssid();
    netMgrTask->restart_wifi();
    change_mode(MODE_MAIN);
    return;
  }

  if ( text == "restart_wifi" ) {
    //    netMgrTask->restart_wifi();
    commonData.msg = "restart_wifi";
    change_mode(MODE_MAIN);
    return;
  }
} // menu_cb()

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

  confFps = new ConfFps();
  confFps->load();
  dispFps = confFps->disp_fps;

  // init commonData
  commonData.netmgr_info = &netMgrInfo;
  commonData.ntp_info = &ntpInfo;

  // BME280
  Bme = new Bme280(BME280_ADDR, TEMP_OFFSET);
  commonData.bme_info = Bme->get();
  float temp_offset = Bme->get_temp_offset();
  log_i("%.1f(%.1f), %.1f, %.1f, %.1f",
        commonData.bme_info->temp, temp_offset,
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

  ntpTask = new NtpTask((String *)NTP_SVR, &netMgrTask, ntp_cb);
  ntpTask->start();
  delay(task_interval);

  netMgrTask = new NetMgrTask("NetMgr", AP_SSID_HDR, &netMgrInfo);
  netMgrTask->start();
  delay(task_interval);

  reBtnWatcher = new ButtonWatcher(RE_BTN_NAME, PIN_BTN_RE, reBtn_cb);
  reBtnWatcher->start();
  delay(task_interval);

  obBtnWatcher = new ButtonWatcher(ONBOARD_BTN_NAME, PIN_BTN_ONBOARD, obBtn_cb);
  obBtnWatcher->start();
  delay(task_interval);

  reWatcher = new RotaryEncoderWatcher(RE_NAME,
                                       PIN_PULSE_DT, PIN_PULSE_CLK,
                                       RE_ANGLE_MAX, RE_LCTRL_MODE,
                                       re_cb);
  reWatcher->start();
  delay(task_interval);

  mqttTask = new MqttTask(&commonData,
                          MQTT_SERVER, MQTT_PORT,
                          MQTT_TOPIC_ROOT,
                          MQTT_CLIENT_ID);
  mqttTask->start();
  delay(task_interval);

  // start timer1
  timer1.attach_ms(TIMER_INTERVAL, timer1_cb);
  log_i("start Timer: %.1f sec", TIMER_INTERVAL / 1000.0);

  // init Mode[]
  mainMode = new MainMode("MainMode", &commonData);
  Mode.push_back(mainMode);

  menuMode = new MenuMode("MenuMode", &commonData, &menu_cb);
  Mode.push_back(menuMode);

  restartMode = new RestartMode("RestartMode", &commonData);
  Mode.push_back(restartMode);

  setTempOffsetMode = new SetTempOffsetMode("SetTempOffsetMode", &commonData);
  Mode.push_back(setTempOffsetMode);

  scanSsidMode = new ScanSsidMode("ScanSsidMode", &commonData);
  Mode.push_back(scanSsidMode);

  setSsidMode = new SetSsidMode("SetSsidMode", &commonData);
  Mode.push_back(setSsidMode);

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

  /*
   * loop
   */
  Mode[curMode]->loop(cur_ms);

  /*
   * display
   */
  Disp->clearDisplay();
  
  if ( commonData.msg.length() > 0 ) {
    log_i("msg:\"%s\"", commonData.msg.c_str());

    if ( commonData.msg == "restart_wifi" ) {
      netMgrTask->restart_wifi();
    }
    
    Disp->fillRect(0, 0, DISPLAY_W, DISPLAY_H, WHITE);
    Disp->setTextColor(BLACK, WHITE);

    Disp->setFont(NULL);
    Disp->setTextSize(1);
    Disp->setCursor(0, 10);
    Disp->setTextWrap(true);
    Disp->printf(" %s", commonData.msg.c_str());

    Disp->display();
    commonData.msg = "";
    delay(1000);

    change_mode(MODE_MAIN);
    return;
  }

  Mode[curMode]->display(Disp);

  // fps
  if ( dispFps ) {
    float fps = 0.0;
    static float min_fps = 99999.9;
    static unsigned long min_fps_ms = millis();
    if ( d_ms != 0 ) {
      fps = 1000.0 / (float)d_ms;
      if ( fps < min_fps ) {
        min_fps = fps;
        min_fps_ms = cur_ms;
      } else if ( cur_ms - min_fps_ms > 3000 ) {
        min_fps = fps;
      }
    }

    int w = 28;
    int h = 7;
    int x = DISPLAY_W - w;
    int y = DISPLAY_H - h - 3;
    Disp->setFont(&Picopixel);
    Disp->setTextSize(1);
    Disp->fillRect(x, y, w, h, BLACK);
    Disp->setCursor(x + 2, y + 5);
    Disp->printf("%4.1fFPS", min_fps);
    Disp->setFont(NULL);
  } // if (dispFps);
  
  Disp->display();
  delay(1);
} // loop()
