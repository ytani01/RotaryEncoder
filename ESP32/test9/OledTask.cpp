/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledTask.h"

static QueueHandle_t OledTask_cmdQue; // XXX

/**
 *
 */
OledTask::OledTask(Esp32RotaryEncoderTask **pReTask,
                   Esp32NetMgrTask **pNetMgrTask,
                   QueueHandle_t in_que)
: Esp32Task("OLED(SSD1306)", 4*1024, 5) {
  this->pReTask = pReTask;
  this->pNetMgrTask = pNetMgrTask;
  OledTask_cmdQue = in_que;
  
  this->disp = NULL;
}

/**
 *
 */
void OledTask::setup() {
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
void OledTask::loop() {
  static bool flag_clr = false;

  portBASE_TYPE ret = xQueueReceive(OledTask_cmdQue, (void *)(this->_buf), 0);
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
  
  Esp32RotaryEncoderTask* reTask = *(this->pReTask);
  if ( reTask != NULL && reTask->is_active() ) {
    int x = DISP_W / 2
      + reTask->re->info.angle * 2
      - reTask->re->info.angle_max * 2 / 2;
    this->disp->fillCircle(x, 34, 10, WHITE);
  }

  this->disp->setTextSize(1);
  this->disp->setCursor(10, 10);
  this->disp->write("SSID:");

  Esp32NetMgrTask *netMgrTask = *(this->pNetMgrTask);
  if ( netMgrTask != NULL ) {
    Esp32NetMgr *netMgr = netMgrTask->netMgr;
    if ( netMgr != NULL ) {
      String ssid = netMgr->cur_ssid;
      String mac_addr = netMgr->get_mac_addr_String();
      
      if ( ssid == "" ) {
        this->disp->write("[No WiFi]");
      } else {
        this->disp->write(ssid.c_str());
      } 

      this->disp->setTextSize(1);
      this->disp->setCursor(10, 50);
      this->disp->write(mac_addr.c_str());
    }
  }

  unsigned long ms0 = millis();
  this->disp->display();
  unsigned long ms = millis() - ms0;
  if ( ms > 30 ) {
    log_w("display(): ms=%d", ms);
  }
} // OledTask::loop()
