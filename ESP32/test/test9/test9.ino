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
    RotaryEncoderInfo_t re_info;
  };
} InData_t;

/**
 * 表示データ
 */
struct {
  String ssid;
  RotaryEncoderInfo_t reInfo;
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
const uint8_t PIN_BTN_ONBOARD = 39;
const uint8_t PIN_BTN_RE = 26;
const String BTN_NAME_ONBOARD = "OnBoard";
const String BTN_NAME_RE = "RotaryEncoder";
Esp32ButtonTask *taskBtnOnboard = NULL;
Esp32ButtonTask *taskBtnRe = NULL;

// Rotary Encoder
const String RE_NAME = "RotaryEncoder1";
const uint8_t PIN_PULSE_DT = 33;
const uint8_t PIN_PULSE_CLK = 32;
const int16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
Esp32RotaryEncoderTask *taskRe = NULL;

// Queues
#define Q_SIZE 32
QueueHandle_t queRe, queBtn, queDispCmd;

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
void ch_hsv(uint8_t hue, uint8_t sat, uint8_t val) {
  leds_onboard[0] = CHSV(hue, sat, val);

  for (int i=0; i < LEDS_N_EXT1; i++) {
    leds_ext1[i] = CHSV(hue, sat, val);
    hue = (hue - 32) % 255;
  }
  
  FastLED.show();
} // ch_hsv();

/**
 * XXX Esp32Task をつかって書き換え
 */
void task1(void *pvParameters) {
  RotaryEncoderInfo_t re_info;
  portBASE_TYPE ret;
  
  while (true) { // main loop
    // get queue
    if ( (ret = xQueueReceive(queRe, (void *)&re_info, 0)) == pdPASS ) {
      // calc color
      uint16_t hue = int(round((float)re_info.angle * 255.0 / (float)PULSE_MAX));
      ch_hsv(hue, 255, 255);
      log_i("que > %s  hue=0x%02X(%5.1f deg)",
            Esp32RotaryEncoder::info2String(re_info).c_str(), hue,
            (float)hue * 360.0 / 256.0);
    } else if ( ret != errQUEUE_EMPTY ) {
      log_e("%d", ret);
    }

    delay(1);
  } // main loop
  vTaskDelete(NULL);
} // task1()

/**
 *
 */
void task_btn_watcher(void *pvParameters) {
  while (true) { // main loop
    Esp32ButtonInfo_t btn_info;
    portBASE_TYPE ret;

    if ( (ret = xQueueReceive(queBtn, (void *)&btn_info, 0)) == pdPASS ) {
      log_i("que > %s", Esp32Button::info2String(btn_info).c_str());

      if ( String(btn_info.name) == BTN_NAME_ONBOARD ) {
        if ( btn_info.long_pressed ) {
          log_w("");
          log_w("restart..");
          log_w("");
          delay(2000);
          //ESP.restart();
          ESP.deepSleep(3000);
          delay(2000);
        }
        delay(10);
        continue;
      }

      if ( String(btn_info.name) == BTN_NAME_RE ) {
        if ( btn_info.push_count > 0 ) {
          ch_hsv(255, 0, 255);
        }

        if ( btn_info.click_count >= 3 ) {
          sprintf(disp_cmd, "clearDisplay%c", '\0');
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
      }
    } // if(xQueueReceive(queBtn)..)

    delay(1);
  } // main loop
  vTaskDelete(NULL);
} // task_btn_watcher()

/**
 *
 */
BaseType_t createTask(TaskFunction_t pxTaskCode,
                      const char * pcName,
                      const uint32_t usStackDepth=1024*8,
                      UBaseType_t uxPriority=1,
                      BaseType_t xCoreID=APP_CPU_NUM
                      // PRO_CPU_NUM:0, APP_CPU_NUM:1
                      ) {
  BaseType_t ret = xTaskCreateUniversal(pxTaskCode, pcName, usStackDepth,
                                        NULL, uxPriority, NULL, xCoreID);
  log_i("Start: %s: ret=%d", pcName, ret);
  if ( ret != pdPASS ) {
    log_e("ret=%d .. HALT", ret);
    while (true) { // !!! halt !!!
      delay(1);
    }
  }
  delay(100);
  log_i("Start: %s: ret=%d", pcName, ret);
  return ret;
} // createTask()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(50);  // Serial Init Wait

  log_d("portTICK_PERIOD_MS=%d", portTICK_PERIOD_MS);
  
  Serial.println("=====");

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
  queRe = xQueueCreate(Q_SIZE, sizeof(RotaryEncoderInfo_t));
  if ( queRe == NULL ) {
    log_e("xQueueCreate(queRe): failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }

  queBtn = xQueueCreate(Q_SIZE, sizeof(Esp32ButtonInfo_t));
  if ( queBtn == NULL ) {
    log_e("xQueueCreate(queBtn): failed .. HALT");
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
  delay(500);

  taskOled = new OledTask(&taskRe, &taskNetMgr, queDispCmd);
  taskOled->start();

  delay(500);

  taskNtp = new Esp32NtpTask((String *)NTP_SVR, &taskNetMgr);
  taskNtp->start();

  delay(500);

  taskNetMgr = new Esp32NetMgrTask("NetMgr", AP_SSID_HDR, WIFI_RETRY_COUNT);
  taskNetMgr->start();

  delay(500);

  createTask(task_btn_watcher, "btn_watcher");

  delay(500);

  createTask(task1, "task1");

  delay(500);

  taskBtnRe = new Esp32ButtonTask(BTN_NAME_RE, PIN_BTN_RE, queBtn);
  taskBtnRe->start();

  delay(500);

  taskBtnOnboard = new Esp32ButtonTask(BTN_NAME_ONBOARD, PIN_BTN_ONBOARD,
                                       queBtn);
  taskBtnOnboard->start();

  delay(500);

  taskRe = new Esp32RotaryEncoderTask(RE_NAME, PIN_PULSE_DT, PIN_PULSE_CLK,
                                      PULSE_MAX, queRe);
  taskRe->start();

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
