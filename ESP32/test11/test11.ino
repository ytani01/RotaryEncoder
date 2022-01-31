/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <Ticker.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include "Esp32NetMgr.h"

#include "Esp32Task.h"
#include "Esp32ButtonTask.h"
#include "Esp32RotaryEncoderTask.h"
#include "OledTask.h"
#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"

/**
 * 入力データ用キューエントリ
 */
typedef enum InType_enum : uint8_t {
                                    INTYPE_BUTTON,
                                    INTYPE_ROTARY_ENCODER
} InType_t;
const char *InTypeSTR[] = {"BUTTON", "ROTARY_ENCODER"};

typedef struct {
  InType_t type;
  union {
    Esp32ButtonInfo_t btn_info;
    Esp32RotaryEncoderInfo_t re_info;
  };
} InData_t;

/**
 * 表示データ
 */
struct {
  String ssid;
  Esp32RotaryEncoderInfo_t reInfo;
  Esp32ButtonInfo_t btnReInfo;
} OutputDate_t;
  
// OLED
const uint16_t DISP_W = 128;
const uint16_t DISP_H = 64;
const uint16_t CH_W = 6;
const uint16_t CH_H = 8;
const uint16_t FRAME_W = 3;
Adafruit_SSD1306 *disp;
char disp_cmd[OledTask::CMD_BUF_SIZE];
OledTask *taskOled = NULL;

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

// Queues
#define Q_SIZE 32
QueueHandle_t queRe, queDispCmd;

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
const unsigned int WIFI_RETRY_COUNT = 10;
Esp32NetMgrTask *taskNetMgr = NULL;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
Esp32NtpTask *taskNtp = NULL;

// Timer
const TickType_t TIMER_INTERVAL = 20 * 1000; // tick == ms (?)
Ticker timer1;

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

  if ( btn_info->click_count > 3 ) {
    portBASE_TYPE ret = xQueueSend(queDispCmd, (void *)disp_cmd, 10);
    if ( ret == pdPASS ) {
      log_i("OK: ret=%d", ret);
    } else {
      log_e("NG: ret=%d", ret);
    }
          
    log_w("restart..");
    delay(3000);
    //ESP.restart();
    ESP.deepSleep(3000);
    delay(2000);
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

  if ( btn_info->click_count > 3 ) {
    portBASE_TYPE ret = xQueueSend(queDispCmd, (void *)disp_cmd, 10);
    if ( ret == pdPASS ) {
      log_i("OK: ret=%d", ret);
    } else {
      log_e("NG: ret=%d", ret);
    }
          
    log_w("restart..");
    delay(3000);
    //ESP.restart();
    ESP.deepSleep(3000);
    delay(2000);
  }
} // obBtn_cb()

/**
 *
 */
void re_cb(Esp32RotaryEncoderInfo_t *re_info) {
  log_i("%s", Esp32RotaryEncoder::info2String(re_info).c_str());
  reInfo = *re_info;
}

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
  log_i("[%s] timer test: end(d_tick=%d)",
        Esp32NtpTask::get_time_str(), d_tick);
}

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(50);  // Serial Init Wait

  Serial.println();
  Serial.println("=====");

  log_i("portTICK_PERIOD_MS=%d", portTICK_PERIOD_MS);
  
  /*
   * XXX test XXX
   */
  Esp32ButtonInfo_t bi1;
  strcpy(bi1.name, "AAA");
  
  InData_t inData;
  inData.type = INTYPE_BUTTON;
  inData.btn_info = bi1;
  log_i("inData.type=%s(%i), %s",
        InTypeSTR[inData.type], inData.type,
        inData.btn_info.name);

  //OutputDate_t outData;

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

  // Queues
  // XXX Esp32RotaryEncoderTask classに入れる
  // XXX Queueにせずに、コールバックを呼び出すほうがいいかも?
  queRe = xQueueCreate(Q_SIZE, sizeof(Esp32RotaryEncoderInfo_t));
  if ( queRe == NULL ) {
    log_e("xQueueCreate(queRe): failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }

  queDispCmd = xQueueCreate(Q_SIZE, sizeof(OledTask::CMD_BUF_SIZE));
  if ( queDispCmd == NULL ) {
    log_e("xQueueCreate(queDispCmd): failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }

  // Tasks
  taskOled = new OledTask(&reBtnInfo, &obBtnInfo, &reInfo,
                          &taskNetMgr, queDispCmd);
  taskOled->start();

  delay(500);

  taskNtp = new Esp32NtpTask((String *)NTP_SVR, &taskNetMgr);
  taskNtp->start();

  delay(500);

  taskNetMgr = new Esp32NetMgrTask("NetMgr", AP_SSID_HDR, WIFI_RETRY_COUNT);
  taskNetMgr->start();

  delay(500);

  reBtnWatcher = new Esp32ButtonWatcher(RE_BTN_NAME, PIN_BTN_RE,
                                        reBtn_cb);
  reBtnWatcher->start();

  delay(500);

  obBtnWatcher = new Esp32ButtonWatcher(ONBOARD_BTN_NAME, PIN_BTN_ONBOARD,
                                        obBtn_cb);
  obBtnWatcher->start();

  delay(500);

  reWatcher = new Esp32RotaryEncoderWatcher(RE_NAME,
                                            PIN_PULSE_DT, PIN_PULSE_CLK,
                                            PULSE_MAX,
                                            re_cb);
  reWatcher->start();

  delay(500);

  // start timer1
  timer1.attach_ms(TIMER_INTERVAL, timer1_cb);
  log_i("start Timer: %.1f sec", TIMER_INTERVAL / 1000.0);
} // setup()

/**
 *
 */
void loop() {
  delay(10);
}
