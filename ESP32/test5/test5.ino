/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <esp_sntp.h>

#undef FASTLED_ALL_PINS_HARDWARE_SPI // for RMT (?)
#undef FASTLED_ESP32_I2S // for RMT (?)
#include <FastLED.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "Esp32PcntRotaryEncoder.h"
#include "NetMgr.h"
#include "Button.h"

// OLED
const uint16_t DISP_W = 128;
const uint16_t DISP_H = 64;
const uint16_t CH_W = 6;
const uint16_t CH_H = 8;
const uint16_t LINE_W = 3;
Adafruit_SSD1306 *disp;

// Buttons
const uint8_t PIN_BTN_ONBOARD = 39;
const uint8_t PIN_BTN_RE = 26;
const String BTN_NAME_ONBOARD = "OnBoard";
const String BTN_NAME_RE = "RotaryEncoder1";
const unsigned long DEBOUNCE = 50; // msec
Button *btnOnboard, *btnRe;

// Rotary Encoder
const String RE_NAME = "RotaryEncoder1";
const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const int16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
Esp32PcntRotaryEncoder *re;

// Queues
#define Q_SIZE 128
QueueHandle_t queRe, queBtn;

// NeoPixel
const uint8_t PIN_NEOPIXEL_ONBOARD = 27;
const uint16_t LEDS_N_ONBOARD = 1;
const uint8_t LED_BRIGHTNESS_ONBOARD = 50;
CRGB leds_onboard[LEDS_N_ONBOARD];

const uint8_t PIN_NEOPIXEL_EXT1 = 18;
const uint16_t LEDS_N_EXT1 = 100;
const uint8_t LED_BRIGHTNESS_EXT1 = 30;
CRGB leds_ext1[LEDS_N_EXT1];

// WiFi
const String AP_SSID_HDR = "test3";
const unsigned int WIFI_RETRY_COUNT = 10;
NetMgr *netMgr;
mode_t netMgrMode;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
const unsigned long NTP_INTERVAL_NORMAL = 5 * 60 * 1000; // ms
const unsigned long NTP_INTERVAL_PROGRESS = 5 * 1000; // ms

// Timer
const TickType_t TIMER_INTERVAL = 60 * 1000; // tick == ms (?)

/**
 *
 */
void IRAM_ATTR re_intr_hdr(void *arg) {
  log_w("int_str=0x%04X, status=0x%04X",
        PCNT.int_st.val, PCNT.status_unit[0].val);

  PCNT.int_clr.val = PCNT.int_st.val;
}

/**
 *
 */
String get_time_str() {
  struct tm ti; // time info
  const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  char buf[4+1+2+1+2 +1+2+1 +1 +2+1+2+1+2 +1];

  getLocalTime(&ti);
  sprintf(buf, "%04d/%02d/%02d(%s) %02d:%02d:%02d",
          ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
          day_str[ti.tm_wday].c_str(),
          ti.tm_hour, ti.tm_min, ti.tm_sec);
  return String(buf);
} // get_time_str()

/**
 * 【注意・重要】
 * コールバック実行中に、次のタイマー時間になると、
 * 次のコールバックが待たされ、あふれるとパニックする。
 */
void timer_cb(TimerHandle_t xTimer) {
  log_i("timer test");
  delay(TIMER_INTERVAL / 2);
  log_i("timer test: end");
} // timer_cb()

/** NTP task function
 *
 */
void task_ntp(void *pvParameters) {
  unsigned long interval;

  setenv("TZ", "JST-9", 1);
  tzset();
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

  while (true) {
    if ( netMgrMode != NetMgr::MODE_WIFI_ON ) {
      delay(1000);
      continue;
    }

    // start sync
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
      Serial.print(".");
      delay(10);
    }
    log_i("sntp_sync_status=%d", sntp_stat);
    if ( sntp_stat == SNTP_SYNC_STATUS_COMPLETED ) {
      interval = NTP_INTERVAL_NORMAL;
    } else {
      interval = NTP_INTERVAL_PROGRESS;
    }
    
    log_i("%s", get_time_str().c_str());

    log_i("interval=%.1f sec", interval / 1000.0);
    delay(interval);
  } // while(true)

  vTaskDelete(NULL);
} // task_ntp()

/** NetMgr task function
 *
 */
void task_net_mgr(void *pvParameters) {
  netMgr = new NetMgr(AP_SSID_HDR, WIFI_RETRY_COUNT);

  while (true) { // main loop
    static mode_t prev_netMgrMode = NetMgr::MODE_NULL;

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
  } // while(true)
  vTaskDelete(NULL);
} // task_net_mgr()

/** Rotary Encoder watcher task function
 *
 */
void task_re_watcher(void *pvParameters) {
  re = new Esp32PcntRotaryEncoder(RE_NAME,
                                  PIN_PULSE_CLK, PIN_PULSE_DT, PULSE_MAX,
                                  PCNT_UNIT,
                                  re_intr_hdr, (void *)NULL);

  while (true) {
    RotaryEncoderAngle_t d_angle = re->get();

    if ( d_angle == 0 ) {
      delay(1);
      continue;
    }
    
    portBASE_TYPE ret;
    while ( (ret = xQueueSend(queRe, (void *)&(re->info), 10)) != pdPASS ) {
      log_e("put queue failed, ret=%d", ret);
      delay(1000);
    }

    delay(10);
  } // while(true)
  vTaskDelete(NULL);
} // task_re_watcher()

/**
 * 
 */
unsigned long hsv2rgb(uint8_t hue, uint8_t sat, uint8_t val) {
  if ( sat == 0 ) {
    unsigned long rgb = (val << 16) + (val << 8) + val;
    return rgb;
  }
  
  unsigned char region = hue / 43;
  unsigned char remainder = (hue - (region * 43)) * 6;

  unsigned char p = (val * (255 - sat)) >> 8;
  unsigned char q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
  unsigned char t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

  uint8_t r, g, b;
  switch (region) {
  case 0:
    r = val;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = val;
    b = p;
    break;
  case 2:
    r = p;
    g = val;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = val;
    break;
  case 4:
    r = t;
    g = p;
    b = val;
    break;
  default:
    r = val;
    g = p;
    b = q;
    break;
  }

  unsigned long rgb = (r << 16) + (g << 8) + b;
  return rgb;
} // hsv2rgb()

/**
 *
 */
void ch_color(uint8_t hue, uint8_t sat, uint8_t val) {
  leds_onboard[0] = CHSV(hue, sat, val);

  for (int i=0; i < LEDS_N_EXT1; i++) {
    leds_ext1[i] = CHSV(hue, sat, val);
    hue = (hue + 32) % 255;
  }
  
  FastLED.show();
} // ch_color();

/**
 *
 */
void task1(void *pvParameters) {
  RotaryEncoderInfo_t re_info;
  portBASE_TYPE ret;
  
  while (true) {
    // get queue
    while ( (ret = xQueueReceive(queRe, (void *)&re_info, 0)) != pdPASS ) {
      delay(1);
    }

    // calc color
    uint16_t hue = int(round((float)re_info.angle * 255.0 / (float)PULSE_MAX));
    log_i("[%s] d_angle=%3d, angle=%02d, hue=0x%02X",
          re_info.name, re_info.d_angle, re_info.angle, hue);
    ch_color(hue, 255, 255);

    delay(1);
  } // while(true)
  vTaskDelete(NULL);
} // task1()

/**
 *
 */
void IRAM_ATTR btn_intr_hdr() {
  Button *btn;

  static unsigned long prev_ms = 0;
  unsigned long cur_ms = millis();

  if ( cur_ms - prev_ms < DEBOUNCE ) {
    return;
  }
  prev_ms = cur_ms;

  btn = NULL;
  if ( btnOnboard->get() ) {
    // log_i("onboard");
    btn = btnOnboard;
  }
  if ( btnRe->get() ) {
    // log_i("rotary encoder");
    btn = btnRe;
  }

  if ( btn != NULL ) {
    portBASE_TYPE ret;
    ButtonInfo_t btn_info = btn->info;
    
    while ( (ret=xQueueSend(queBtn, (void *)&(btn_info), 10)) != pdPASS ) {
      log_w("put queue failed:ret=%d", ret);
      delay(1000);
    }
    log_i("send que: %s", Button::info2String(btn->info, true).c_str());
  }
} // btn_intr_hdr()

/**
 *
 */
void task_btn_watcher(void *pvParameters) {

  while (true) { // main loop
    ButtonInfo_t btn_info;
    portBASE_TYPE ret;
    Button *btn;

    btn = NULL;
    if ( btnOnboard->get() ) {
      btn = btnOnboard;
    }
    if ( btnRe->get() ) {
      btn = btnRe;
    }
    if ( btn != NULL ) {
      btn_info = btn->info;

      while ( (ret = xQueueSend(queBtn, (void *)&btn_info, 10)) != pdPASS ) {
        log_w("put queue failed");
        delay(1);
      }
      log_i("send que: %s", Button::info2String(btn->info).c_str());
      delay(1);
      continue;
    }

    if ( (ret = xQueueReceive(queBtn, (void *)&btn_info, 0)) == pdPASS ) {
      log_i("recv que: %s", Button::info2String(btn_info).c_str());

      if ( String(btn_info.name) == BTN_NAME_ONBOARD ) {
        if ( btn_info.long_pressed ) {
          log_w("restart..");
          delay(2000);
          //ESP.restart();
          ESP.deepSleep(3000);
          delay(2000);
        }
        delay(10);
        continue;
      }

      if ( String(btn_info.name) == BTN_NAME_RE ) {
        if ( btn_info.count > 0 ) {
          ch_color(255, 255, 255);
        }
      }
    } // if(xQueueReceive(queBtn)..)

    delay(1);
  } // while(true)
  vTaskDelete(NULL);
} // task_btn_watcher()

/**
 *
 */
BaseType_t createTask(TaskFunction_t pxTaskCode,
                      const char * pcName,
                      const uint16_t usStackDepth=0x2000,
                      UBaseType_t uxPriority=1
                      ) {
  log_i("Start: %s", pcName);
  BaseType_t ret = xTaskCreateUniversal(pxTaskCode, pcName, usStackDepth,
                                        NULL, uxPriority, NULL, APP_CPU_NUM);
  delay(10);
  return ret;
} // createTask()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(50);  // Serial Init Wait

  Serial.println("=====");

  // Buttons
  btnOnboard = new Button(BTN_NAME_ONBOARD, PIN_BTN_ONBOARD, btn_intr_hdr);
  btnRe = new Button(BTN_NAME_RE, PIN_BTN_RE, btn_intr_hdr);
  
  // OLED
  disp = new Adafruit_SSD1306(DISP_W, DISP_H);
  if (!disp->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    log_e("SSD1306: init failed");
    while (true) {
      delay(1);
    }
  }

  disp->display(); // display Adafruit Logo
  delay(2000);
  disp->clearDisplay();
  disp->setTextColor(WHITE);
  disp->setTextWrap(false);

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
    log_e("xQueueCreate: failed");
    while(true) {
      delay(1);
    }
  }

  queBtn = xQueueCreate(Q_SIZE, sizeof(ButtonInfo_t));
  if ( queBtn == NULL ) {
    log_e("xQueueCreate: failed");
    while(true) {
      delay(1);
    }
  }

  // Tasks
  createTask(task_re_watcher, "re_watcher");
  createTask(task_btn_watcher, "btn_watcher");
  createTask(task_net_mgr, "net_mgr");
  createTask(task_ntp, "task_ntp");
  createTask(task1, "task1");

  // Timer
  TimerHandle_t timer1 = xTimerCreate("TIMER1", TIMER_INTERVAL,
                                      pdTRUE, NULL,
                                      timer_cb);
  xTimerStart(timer1, 0);
  log_i("start Timer: %.1f sec", TIMER_INTERVAL / 1000.0);
}

/**
 *
 */
void loop() {
  // XXX ここの処理は、別途タスクにすべき XXX
  disp->clearDisplay();

  disp->fillRect(0,0, DISP_W, DISP_H, WHITE);
  disp->fillRect(LINE_W, LINE_W,
                 DISP_W - LINE_W * 2, DISP_H - LINE_W * 2,
                 BLACK);

  disp->drawRect(10,10,10,10,WHITE);
  disp->drawRect(100,30,20,20,WHITE);
  disp->fillCircle(DISP_W / 2 + re->info.angle * 2 - re->info.angle_max * 2 / 2,
                   32, 20, WHITE);
  disp->display();
  
  delay(10);
}
