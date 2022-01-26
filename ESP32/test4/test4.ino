/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include <sntp.h>

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
const uint16_t LINE_W = 1;
Adafruit_SSD1306 *disp;

// Buttons
const uint8_t PIN_BTN_ONBOARD = 39;
const uint8_t PIN_BTN_RE = 26;
const unsigned long DEBOUNCE = 50; // msec
Button *btnOnboard, *btnRe;

// Rotary Encoder
const uint8_t PIN_PULSE_DT = 32;
const uint8_t PIN_PULSE_CLK = 33;
const uint16_t PULSE_MAX = 30;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

// Queues
#define Q_SIZE 64
QueueHandle_t queRe, queBtn;

// NeoPixel
const uint8_t PIN_NEOPIXEL = 27;
const uint16_t NUM_LEDS = 1;
const uint8_t LED_BRIGHTNESS = 50;
CRGB leds[NUM_LEDS];

// WiFi
const String AP_SSID_HDR = "test3";
const unsigned int WIFI_RETRY_COUNT = 10;
NetMgr *netMgr;
mode_t netMgrMode;

// NTP
const String NTP_SVR[] = {"ntp.nict.jp", "pool.ntp.org", "time.google.com"};
const TickType_t NTP_INTERVAL = 1 * 1000; // tick == ms (?)
volatile static bool ntp_syncing = false;

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
void timer_cb_ntp(TimerHandle_t xTimer) {
  if ( netMgrMode != NetMgr::MODE_WIFI_ON ) {
    return;
  }

  if ( ntp_syncing ) {
    log_w("NTP SYNCING");
    return;
  }
  ntp_syncing = true;

  struct tm ti; // time info
  const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  
  configTime(9 * 3600L, 0,
             NTP_SVR[0].c_str(), NTP_SVR[0].c_str(), NTP_SVR[0].c_str());

  sntp_sync_mode_t sync_mode = sntp_get_sync_mode();
  log_i("sync_mode=%d", sync_mode);

  sntp_sync_status_t sync_stat;
  while ( (sync_stat = sntp_get_sync_status()) == SNTP_SYNC_STATUS_RESET ) {
    log_i("sync_stat=%d", sync_stat);
    delay(1000);
  }
  log_i("sync_stat=%d", sync_stat);

  getLocalTime(&ti);
  log_i("%04d/%02d/%02d(%s) %02d:%02d:%02d",
        ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
        day_str[ti.tm_wday].c_str(),
        ti.tm_hour, ti.tm_min, ti.tm_sec);

  ntp_syncing = false;
}

/** NetMgr task function
 *
 */
void task_net_mgr(void *pvParameters) {
  netMgr = new NetMgr(AP_SSID_HDR, WIFI_RETRY_COUNT);

  TimerHandle_t ntp_timer = xTimerCreate("NTP", NTP_INTERVAL , pdTRUE, NULL,
                                         timer_cb_ntp);
  xTimerStart(ntp_timer, 0);
  
  while (true) { // main loop
    static mode_t prev_netMgrMode = NetMgr::MODE_NULL;

    netMgrMode = netMgr->loop();
    if ( netMgrMode != prev_netMgrMode ) {
      log_i("netMgrMode=0x%02X", netMgrMode);
      prev_netMgrMode = netMgrMode;
    }

    delay(1);
  } // while(true)
} // task_net_mgr()

/** Rotary Encoder watcher task function
 *
 */
void task_re_watcher(void *pvParameters) {
  Esp32PcntRotaryEncoder *re;

  re = new Esp32PcntRotaryEncoder(PIN_PULSE_CLK, PIN_PULSE_DT,
                                  PCNT_UNIT, PULSE_MAX,
                                  re_intr_hdr, (void *)NULL);

  while (true) {
    int16_t d_angle = re->get();

    if ( d_angle == 0 ) {
      delay(1);
      continue;
    }
    
    int16_t angle = re->angle;

    portBASE_TYPE ret;

    while ( (ret = xQueueSend(queRe, (void *)&angle, 10)) != pdPASS ) {
      log_w("put queue failed");
      delay(1);
    }

    delay(20);
  } // while(true)
}

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
}

/**
 *
 */
void ch_color(uint8_t r, uint8_t g, uint8_t b) {
  leds[0] = CRGB(r, g, b);
  FastLED.show();
} // ch_color();

/**
 *
 */
void task1(void *pvParameters) {
  int16_t angle;
  portBASE_TYPE ret;
  
  while (true) {
    // get queue
    while ( (ret = xQueueReceive(queRe, (void *)&angle, 10)) != pdPASS ) {
      delay(1);
    }

    // calc color
    uint16_t hue = int(round((float)angle * 255.0 / (float)PULSE_MAX));
    uint8_t sat = 255;
    uint8_t val = 255;

    unsigned long rgb = hsv2rgb(hue, sat, val);
    log_i("angle=%02d, HSV=0x%02X,0x%02X,0x%02X, rgb=#%06X",
          angle, hue, sat, val, rgb);

    uint8_t r = (rgb & 0xff0000) >> 16;
    uint8_t g = (rgb & 0x00ff00) >> 8;
    uint8_t b = (rgb & 0x0000ff);

    ch_color(r, g, b);

    delay(1);
  } // while(true)
} // task1()

/**
 *
 */
void IRAM_ATTR btn_intr_hdr() {
  Button *btn;

  static unsigned long prev_ms = 0;
  unsigned long cur_ms = millis();

  if ( cur_ms - prev_ms < 50 ) {
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
      delay(1);
    }
    log_i("%s", Button::info2String(btn->info, true).c_str());
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

      if ( btn_info.pin == PIN_BTN_ONBOARD ) {
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

      if ( btn_info.pin == PIN_BTN_RE ) {
        ch_color(255, 255, 255);
      }
    }

    delay(1);
  } // while(true)
} // task_btn_watcher()

/**
 *
 */
BaseType_t createTask(TaskFunction_t pxTaskCode,
                      const char * pcName,
                      const uint16_t usStackDepth=0x2000) {
  return xTaskCreateUniversal(pxTaskCode, pcName, usStackDepth,
                              NULL, 1, NULL, APP_CPU_NUM);
} // createTask()

/**
 *
 */
void setup() {
  Serial.begin(115200);
  delay(50);  // Serial Init Wait

  btnOnboard = new Button(PIN_BTN_ONBOARD, "Onboard", btn_intr_hdr);
  btnRe = new Button(PIN_BTN_RE, "RotaryEncoder", btn_intr_hdr);
  
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

  FastLED.addLeds<WS2812B, PIN_NEOPIXEL, GRB>
    (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS);
  leds[0] = CRGB(255,255,255);
  FastLED.show();

  queRe = xQueueCreate(Q_SIZE, sizeof(int16_t));
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

  createTask(task_re_watcher, "re_watcher");
  createTask(task_btn_watcher, "btn_watcher");
  createTask(task_net_mgr, "net_mgr");
  createTask(task1, "task1");
}

/**
 *
 */
void loop() {
  /*
  disp->fillRect(0,0, DISP_W-1, DISP_H-1, WHITE);
  disp->fillRect(LINE_W, LINE_W,
                 DISP_W - LINE_W * 4, DISP_H - LINE_W * 4,
                 BLACK);
  */
  disp->clearDisplay();
  disp->drawRect(10,10,10,10,WHITE);
  disp->drawRect(100,30,20,20,WHITE);
  disp->fillCircle(64, 32, 20, WHITE);
  disp->display();
  
  delay(1);
}
