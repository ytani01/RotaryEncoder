/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledTask.h"
#include <Adafruit_BME280.h>

#define _D this->disp

/**
 * OLED(SSD1306)と、BME280は、同一タスクにしないとハマる!!??
 */
Adafruit_BME280 bme;
bool bme_active = false;

/**
 *
 */
OledTask::OledTask(DispData_t *disp_data):
  Esp32Task("OLED(SSD1306)", 4*1024, 1) {

  this->disp_data = disp_data;
  _D = NULL;
  this->_bme = NULL;
}

/**
 *
 */
void OledTask::setup() {
  this->_bme = new Esp32Bme280(0x76, -1);

  _D = new Adafruit_SSD1306(DISP_W, DISP_H);
  _D->begin(SSD1306_SWITCHCAPVCC, 0x3C);

  _D->display(); // display Adafruit Logo
  delay(1000);

  _D->clearDisplay();
  //_D->cp437(true);
  _D->setTextColor(WHITE);
  _D->setTextWrap(true);
  _D->cp437(true);

} // OledTask::setup()

/**
 *
 */
void OledTask::drawTemp(int x, int y, float temp) {
  _D->setCursor(x, y);
  _D->setTextSize(3);
  _D->printf("%2.0f", temp);
  _D->setCursor(x + CH_W * 3 * 2, y);
  _D->setTextSize(1);
  _D->printf("%cC ", (char)0xF8);
  _D->setCursor(x + CH_W * 2 * 3 - 5, y + CH_H);
  _D->setTextSize(2);
  _D->printf(".%d", int((temp - int(temp))*10));
} // OledTask::drawTemp()

/**
 *
 */
void OledTask::drawHum(int x, int y, float hum) {
  _D->setCursor(x, y);
  _D->setTextSize(2);
  _D->printf("%2.0f", hum);
  _D->setCursor(x + CH_W * 2 * 2, y + CH_H - 1);
  _D->setTextSize(1);
  _D->printf("%%");  
} // OledTask::drawHum()

/**
 *
 */
void OledTask::drawPres(int x, int y, float pres) {
  _D->setCursor(x, y);
  _D->setTextSize(1);
  _D->printf("%4.0f", pres);

  _D->setCursor(x + CH_W * 4 + 3, y);
  _D->printf("hPa");
} // OledTask::drawPres()

/**
 *
 */
void OledTask::drawThi(int x, int y, float thi) {
  _D->setCursor(x, y);
  _D->setTextSize(2);
  _D->printf("%2.0f", thi);

  _D->setCursor(x, y + CH_H * 2);
  _D->setTextSize(1);
  _D->printf(" THI");
} // OledTask::drawThi()

/**
 *
 */
void OledTask::drawWiFi(int x, int y, Esp32NetMgrInfo_t *ni) {
  _D->setCursor(x, y);
  _D->setTextSize(1);
  if ( ni->mode == NETMGR_MODE_WIFI_ON ) {
    _D->printf("SSID:%s", ni->ssid.c_str());
  } else if ( ni->mode == NETMGR_MODE_AP_LOOP ) {
    _D->printf("AP:%s", ni->ssid.c_str());
  } else {
    _D->printf("WiFi..");
  }  
} // OledTask::drawWiFi()

/**
 *
 */
void OledTask::drawDateTime(int x, int y, struct tm *ti) {
  int mon_x = x - 2;
  int mon_y = y;
  int hour_x = mon_x + CH_W * 2 * 5 + 1;
  int hour_y = mon_y + 1;
  int sec_x = x + 53;
  int sec_y = y + 18;

  int x1 = mon_x + CH_W * 2 * 2;
  int y1 = mon_y + CH_H * 2 - 4;
  int x2 = x1 + CH_W - 2;
  int y2 = y1 - CH_H * 2 + 6;
  _D->drawLine(x1, y1, x2, y2, WHITE);

  if ( millis() % 1000 >= 500 ) {
    _D->setCursor(hour_x + CH_W * 2 * 2 - 4, hour_y);
    _D->setTextSize(2);
    _D->printf(":");
  }

  _D->drawRect(sec_x, sec_y, 61, 5, WHITE);
  for (int x1=sec_x; x1 <= sec_x+60; x1 += 10) {
    _D->drawFastVLine(x1, sec_y, 3, WHITE);
  }
  
  if ( ti->tm_year + 1900 < 2000 ) {
    return;
  }

  char year_str[8], month_str[4], day_str[4], wday_str[4];
  char hour_str[4], minute_str[4];
  strftime(wday_str, sizeof(wday_str), "%a", ti);
  
  x = mon_x;
  y = mon_y;
  _D->setCursor(mon_x, mon_y);
  _D->setTextSize(2);
  //_D->write(year_str);
  _D->printf("%2d\n", ti->tm_mon + 1);

  x += CH_W * (2 * 2 + 1);
  _D->setCursor(x, y);
  _D->setTextSize(2);
  _D->printf("%-2d", ti->tm_mday);

  y += CH_H * 2;
  _D->setCursor(x, y);
  _D->setTextSize(1);
  _D->printf("%s", wday_str);

  x = hour_x;
  y = hour_y;
  _D->setCursor(x, y);
  _D->setTextSize(2);
  _D->printf("%02d", ti->tm_hour);

  _D->setCursor(x + CH_W * 2 * 2 + 4, y);
  _D->printf("%02d", ti->tm_min);

  _D->fillRect(sec_x+1, sec_y, ti->tm_sec, 4, WHITE);
} // OledTask::drawDateTime()

/**
 *
 */
void OledTask::loop() {
  static unsigned long prev_ms = millis();
  static int d_ms_max = 0;
  static unsigned long d_ms_max_ms = millis();

  unsigned long cur_ms = millis();
  int d_ms = cur_ms - prev_ms;
  prev_ms = cur_ms;

  // msec per display
  static float fps = 0.0;
  if ( d_ms >= d_ms_max || cur_ms - d_ms_max_ms >= 3000 ) {
    if ( d_ms != d_ms_max ) {
      log_d("d_ms=%d %d:d_ms_max=%d",
            d_ms, cur_ms - d_ms_max_ms, d_ms_max);
      d_ms_max = d_ms;
    }
    d_ms_max_ms = cur_ms;
    if ( d_ms_max == 0 ) {
      fps = 0.0;
    } else {
      fps = 1000.0 / (float)d_ms_max;
    }
  }

  // cmd
  String cmd = String(disp_data->cmd);
  if ( cmd != "" ) {
    log_i("cmd=%s", disp_data->cmd);
  }
  static bool clr_flag = false;
  if ( cmd == "clear" ) {
    clr_flag = true;
    strcpy(disp_data->cmd, "");
  }

  // get temp, hum, pres
  static float temp, hum, pres, thi;
  static unsigned long prev_temp_ms = millis() - 5000;
  if ( cur_ms - prev_temp_ms > 10 * 1000 ) {
    prev_temp_ms = cur_ms;
    Esp32Bme280Info_t bme_info;
    if ( this->_bme->is_active() ) {
      this->_bme->get(&bme_info);
      temp = bme_info.temp;
      hum = bme_info.hum;
      pres = bme_info.pres;
      thi = bme_info.thi;
      log_i("%.1f C %.0f%% %0.0f hPa %.1f thi", temp, hum, pres, thi);
    }
  }

  Esp32NetMgrInfo_t *ni = this->disp_data->ni;
  Esp32ButtonInfo_t *bi1 = this->disp_data->bi1;
  Esp32RotaryEncoderInfo_t *ri1 = this->disp_data->ri1;
  Esp32NtpTaskInfo_t *ntp_info = this->disp_data->ntp_info;

  String ntp_stat_str = "?";
  switch ( ntp_info->sntp_stat ) {
  case SNTP_SYNC_STATUS_RESET:
    ntp_stat_str = "!";
    break;
  case SNTP_SYNC_STATUS_IN_PROGRESS:
    ntp_stat_str = "*";
    break;
  case SNTP_SYNC_STATUS_COMPLETED:
    ntp_stat_str = "=";
    break;
  default:
    break;
  } // switch (sntp_stat)
  if ( ni->mode != NETMGR_MODE_WIFI_ON ) {
    ntp_stat_str = "x";
  }

  time_t t = time(NULL);
  struct tm *ti = localtime(&t);

  /**
   * draw, fill ..
   */
  int x, y, r;
  // clear
  _D->clearDisplay();

  _D->drawFastHLine(0, 25, DISP_W - 1, WHITE);
  _D->drawFastHLine(0, 53, DISP_W - 1, WHITE);

  if ( clr_flag ) {
    _D->display();
    return;
  }
  clr_flag = false;
  
  // msec per display
  x = DISP_W - CH_W * 4 - 2;
  y = DISP_H - CH_H;
  _D->setCursor(x, y);
  _D->setTextSize(1);
  _D->printf("%.1f", fps);

  // cmd
  if ( cmd != "" ) {
    x = 82;
    y = 2;
    _D->setCursor(x, y);
    _D->setTextSize(1);
    _D->printf("%s", cmd.c_str());
  }
  
  x = 0;
  y = 0;
  this->drawTemp(x, y, temp);

  x += 63;
  this->drawHum(x, y, hum);

  x -= 5;
  y += CH_H * 2;
  this->drawPres(x, y, pres);

  x = DISP_W - CH_W * 2 * 2;
  y = 0;
  this->drawThi(x, y, thi);
  
  x = 0;
  y = 28;
  this->drawDateTime(x, y, ti);

  x = DISP_W - CH_W * 2;
  y = 30;
  _D->setCursor(x, y);
  _D->setTextSize(2);
  _D->printf("%s", ntp_stat_str.c_str());
  
  x = 0;
  y = DISP_H - CH_H;
  this->drawWiFi(x, y, ni);

  // Circle
  if ( ri1->angle_max != 0 ) {
    x = DISP_W * ri1->angle / ri1->angle_max;
    y = DISP_H - bi1->push_count * 4;
    r = 3 + bi1->repeat_count * 2;
    _D->fillCircle(x, y, r, WHITE);
  }

  _D->display();
  if ( d_ms_max > 35 ) {
    log_w("display(): d_ms_max=%d", d_ms);
  }
} // OledTask::loop()
