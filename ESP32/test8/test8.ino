/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <esp_sntp.h>
#include <esp_task_wdt.h>
#include <Ticker.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "NetMgr.h"

#include "Esp32Task.h"
#include "Esp32OledTask.h"
#include "Esp32ButtonTask.h"
#include "Esp32RotaryEncoderTask.h"

#include "task_test1.h"

/**
 * 表示データ
 */
struct {
  String ssid;
  RotaryEncoderInfo_t reInfo;
  Esp32ButtonInfo_t btnReInfo;
} OutputDate_t;
  
/**
 * 入力データ用キューエントリ
 */
typedef struct {
  enum {
        BUTTON,
        ROTARY_ENCODER
  } type;
  union {
    Esp32ButtonInfo_t bi;
    RotaryEncoderInfo_t ri;
  };
} InputData_t;

// OLED
const uint16_t DISP_W = 128;
const uint16_t DISP_H = 64;
const uint16_t CH_W = 6;
const uint16_t CH_H = 8;
const uint16_t FRAME_W = 3;
Adafruit_SSD1306 *disp;
char disp_cmd[Esp32OledTask::CMD_BUF_SIZE];

// Esp32Buttons
const uint8_t PIN_BTN_ONBOARD = 39;
const uint8_t PIN_BTN_RE = 26;
const String BTN_NAME_ONBOARD = "OnBoard";
const String BTN_NAME_RE = "RotaryEncoder";

// Rotary Encoder
const String RE_NAME = "RotaryEncoder1";
const uint8_t PIN_PULSE_DT = 33;
const uint8_t PIN_PULSE_CLK = 32;
const int16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

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
NetMgr *netMgr = NULL;
mode_t netMgrMode;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
const unsigned long NTP_INTERVAL_NORMAL = 5 * 60 * 1000; // ms
const unsigned long NTP_INTERVAL_PROGRESS = 5 * 1000; // ms

// Timer
const TickType_t TIMER_INTERVAL = 20 * 1000; // tick == ms (?)
Ticker timer1;

// Tasks
Esp32OledTask *taskOled = NULL;
Esp32RotaryEncoderTask *taskRe = NULL;
Esp32ButtonTask *taskBtnOnboard = NULL;
Esp32ButtonTask *taskBtnRe = NULL;

/**
 *
 */
String get_time_String() {
  struct tm ti; // time info
  const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  char buf[4+1+2+1+2 +1+3+1 +1 +2+1+2+1+2 +1];

  getLocalTime(&ti);
  strftime(buf, sizeof(buf), "%Y-%m-%d(%a) %H:%M:%S", &ti);
  return String(buf);
} // get_time_String()

/**
 * 【注意・重要】
 * コールバック実行中に、次のタイマー時間になると、
 * 次のコールバックが待たされ、あふれるとパニックする。
 */
void timer_cb(TimerHandle_t xTimer) {
  TickType_t tick = xTaskGetTickCount();
  vTaskPrioritySet(NULL, 0);
  log_i("timer test: priority=%d, tick=%d", uxTaskPriorityGet(NULL), tick);

  delay(TIMER_INTERVAL / 2);

  log_i("timer test: end");
} // timer_cb()

/**
 *
 */
void timer1_cb() {
  TickType_t tick1 = xTaskGetTickCount();
  log_i("[%s] timer test: start(priority=%d)",
        get_time_String().c_str(), tick1, uxTaskPriorityGet(NULL));

  delay(TIMER_INTERVAL / 2);

  TickType_t tick2 = xTaskGetTickCount();
  TickType_t d_tick = tick2 - tick1;
  log_i("[%s] timer test: end(d_tick=%d)",
        get_time_String().c_str(), tick2, d_tick);
}

/** NTP task function
 *
 */
void task_ntp(void *pvParameters) {
  unsigned long interval;

  setenv("TZ", "JST-9", 1);
  tzset();
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

  while (true) { // main loop
    if ( netMgrMode != NetMgr::MODE_WIFI_ON ) {
      delay(500);
      continue;
    }

    // start sync
    log_i("start sync ..");
    configTime(9 * 3600L, 0,
               NTP_SVR[0].c_str(), NTP_SVR[0].c_str(), NTP_SVR[0].c_str());

    /*
     * sntp_get_sync_status()
     *   同期未完了の場合、SNTP_SYNC_STATUS_RESET
     *   動機が完了すると「一度だけ」、SNTP_SYNC_STATUS_COMPLETE
     *   SNTP_SYNC_MODE_SMOOTHの同期中の場合は、SNTP_SYNC_STAUS_IN_PROGRESS)
     */
    sntp_sync_status_t sntp_stat;
    while ( (sntp_stat = sntp_get_sync_status()) == SNTP_SYNC_STATUS_RESET) {
      // XXX 回数制限すべき XXX
      log_i("NTP syncing .. : sntp_stat=%d", sntp_stat);
      delay(500);
    }
    if ( sntp_stat == SNTP_SYNC_STATUS_COMPLETED ) {
      interval = NTP_INTERVAL_NORMAL;
      log_i("%s: NTP sync done: sntp_stat=%d, interval=%d",
            get_time_String().c_str(), sntp_stat, interval);
    } else if ( sntp_stat == SNTP_SYNC_STATUS_IN_PROGRESS ) {
      interval = NTP_INTERVAL_PROGRESS;
      log_i("%s: NTP sync progress: sntp_stat=%d, interval=%d",
            get_time_String().c_str(), sntp_stat, interval);
    } else {
      interval = NTP_INTERVAL_PROGRESS;
      log_i("%s: NTP sync failed(?): sntp_stat=%d, interval=%d",
            get_time_String().c_str(), sntp_stat, interval);
    }

    delay(interval);
  } // main loop
  vTaskDelete(NULL);
} // task_ntp()

/** NetMgr task function
 *
 */
void task_net_mgr(void *pvParameters) {
  // Watchdog Timer の初期化
  ESP_ERROR_CHECK(esp_task_wdt_init(60, true));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  netMgr = new NetMgr(AP_SSID_HDR, WIFI_RETRY_COUNT);

  while (true) { // main loop
    static mode_t prev_netMgrMode = NetMgr::MODE_NULL;

    ESP_ERROR_CHECK(esp_task_wdt_reset());

    netMgrMode = netMgr->loop();
    if ( netMgrMode != prev_netMgrMode ) {
      log_i("netMgrMode=0x%02X", netMgrMode);

      if ( netMgrMode == NetMgr::MODE_WIFI_ON ) {
        delay(1000);
        configTime(9 * 3600L, 0,
                   NTP_SVR[0].c_str(), NTP_SVR[0].c_str(), NTP_SVR[0].c_str());
      }
      
      prev_netMgrMode = netMgrMode;
    }

    delay(1);
  } // main loop
  vTaskDelete(NULL);
} // task_net_mgr()

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
  
  /*
   * XXX test XXX
   */
  Esp32ButtonInfo_t bi1;
  RotaryEncoderInfo_t ri1;

  InputData_t inData;
  inData.type = BUTTON;
  inData.bi = bi1;
  inData.ri = ri1;

  OutputDate_t outData;

  Serial.println("=====");

  // Onboard NeoPixel
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

  queDispCmd = xQueueCreate(Q_SIZE, sizeof(Esp32OledTask::CMD_BUF_SIZE));
  if ( queDispCmd == NULL ) {
    log_e("xQueueCreate(queDispCmd): failed .. HALT");
    while (true) { // HALT
      delay(1);
    }
  }

  /*
   * Tasks
   */
  log_i("OledTask");
  taskOled = new Esp32OledTask(&taskRe, &netMgr, queDispCmd);
  taskOled->start();
  delay(500);

  createTask(task_net_mgr, "net_mgr", 4 * 1024); //?
  delay(500);
  createTask(task_ntp, "task_ntp");
  delay(500);

  createTask(task_btn_watcher, "btn_watcher");
  delay(500);
  createTask(task1, "task1");
  delay(500);

  log_i("%s", BTN_NAME_RE.c_str());
  taskBtnRe = new Esp32ButtonTask(BTN_NAME_RE, PIN_BTN_RE, queBtn);
  taskBtnRe->start();
  delay(500);

  log_i("%s", BTN_NAME_ONBOARD.c_str());
  taskBtnOnboard = new Esp32ButtonTask
    (BTN_NAME_ONBOARD, PIN_BTN_ONBOARD, queBtn);
  taskBtnOnboard->start();
  delay(500);

  log_i("%s", RE_NAME.c_str());
  taskRe = new Esp32RotaryEncoderTask
    (RE_NAME, PIN_PULSE_DT, PIN_PULSE_CLK, PULSE_MAX, queRe);
  taskRe->start();
  delay(500);

  log_i("testTask1");
  testTask1 *t1 = new testTask1();
  t1->start();
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