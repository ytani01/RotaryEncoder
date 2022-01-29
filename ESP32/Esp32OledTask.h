/**
 * Copyright (c) Yoichi Tanibayashi
 */
#include "Esp32Task.h"
#include "Esp32PcntRotaryEncoder.h"
#include "NetMgr.h"

static QueueHandle_t Esp32OledTask_cmdQue; // XXX

/**
 *
 */
class Esp32OledTask: public Esp32Task {
 public:
  static const uint16_t DISP_W = 128;
  static const uint16_t DISP_H = 64;
  static const uint16_t CH_W = 6;
  static const uint16_t CH_H = 8;
  static const uint16_t FRAME_W = 3;

  static const uint32_t CMD_BUF_SIZE = 128;
  static const uint32_t CMD_NAME_SIZE = 8;

  Adafruit_SSD1306 *disp;

  Esp32PcntRotaryEncoder **re;
  NetMgr **netMgr;

  Esp32OledTask(Esp32PcntRotaryEncoder **re, NetMgr **net_mgr,
                QueueHandle_t in_que);

  virtual void setup();
  virtual void loop();

 private:
  char _buf[CMD_BUF_SIZE];
  char _cmd[CMD_NAME_SIZE];
};

/**
 *
 */
Esp32OledTask::Esp32OledTask(Esp32PcntRotaryEncoder **re,
                             NetMgr **net_mgr,
                             QueueHandle_t in_que)
: Esp32Task("OLED(SSD1306)", 4*1024, 5) {
  this->re = re;
  this->netMgr = net_mgr;
  Esp32OledTask_cmdQue = in_que;
  
  this->disp = NULL;
}

/**
 *
 */
void Esp32OledTask::setup() {
  this->disp = new Adafruit_SSD1306(DISP_W, DISP_H);
  this->disp->begin(SSD1306_SWITCHCAPVCC, 0x3C);

  this->disp->display(); // display Adafruit Logo
  delay(1000);

  this->disp->clearDisplay();
  this->disp->setTextColor(WHITE);
  this->disp->setTextWrap(false);
  this->disp->cp437(true);
}

/**
 *
 */
void Esp32OledTask::loop() {
  static bool flag_clr = false;

  portBASE_TYPE ret = xQueueReceive(Esp32OledTask_cmdQue, (void *)(this->_buf), 0);
  if ( ret == pdPASS ) {
    log_i("que < %s", this->_buf);
    flag_clr = true;
  } else if ( ret != errQUEUE_EMPTY ) {
    log_e("recv que failed: ret=%d", ret);
  }
  
  this->disp->clearDisplay();

  if ( flag_clr ) {
    log_i("clear");
    this->disp->display();
    delay(500);
    return;
  }

  this->disp->fillRect(0,0, DISP_W, DISP_H, WHITE);
  this->disp->fillRect(FRAME_W, FRAME_W,
                 DISP_W - FRAME_W * 2, DISP_H - FRAME_W * 2,
                 BLACK);

  this->disp->drawRect(100,30,20,20,WHITE);
  if ( *(this->re) != NULL ) {
    int x = DISP_W / 2
      + (*(this->re))->info.angle * 2
      - (*(this->re))->info.angle_max * 2 / 2;
    this->disp->fillCircle(x, 32, 15, WHITE);
  }

  this->disp->setTextSize(1);
  this->disp->setCursor(10, 10);
  this->disp->write("SSID:");

  if ( (*(this->netMgr)) != NULL ) {
    if ( (*(this->netMgr))->cur_ssid == "" ) {
      this->disp->write("[No WiFi]");
    } else {
      this->disp->write((*(this->netMgr))->cur_ssid.c_str());
    }
    this->disp->setTextSize(1);
    this->disp->setCursor(10, 50);
    this->disp->write((*(this->netMgr))->get_mac_addr_String().c_str());
  }

  unsigned long ms0 = millis();
  this->disp->display();
  unsigned long ms = millis() - ms0;
  if ( ms > 30 ) {
    log_w("display(): ms=%d", ms);
  }

  delay(5);
}
