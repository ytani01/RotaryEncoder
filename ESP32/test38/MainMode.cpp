/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "MainMode.h"

/** constructor
 *
 */
MainMode::MainMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

} // MainMode::MainMode()

/**
 *
 */
Mode_t MainMode::reBtn_cb(ButtonInfo_t *bi) {
  if ( bi->click_count > 0 ) {
    return MODE_MENU;
  }
  return MODE_N;
} // MainMode::reBtn_cb()

/**
 *
 */
Mode_t MainMode::obBtn_cb(ButtonInfo_t *bi) {
  if ( bi->click_count > 0 ) {
    common_data->msg = " Onboard Btn\n";
    common_data->msg += " click:" + String(bi->click_count);
  }
  return MODE_N;
} // MainMode::obBtn_cb()

/**
 *
 */
Mode_t MainMode::re_cb(RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle != 0 ) {
    return MODE_MENU;
  }

  return MODE_N;
} // MainMode::re_cb()

/**
 *
 */
void MainMode::display(Display_t *disp) {
  int x, y;

  //disp->drawRect(0,0, DISPLAY_W, DISPLAY_H, WHITE);
  disp->setTextWrap(false);
  disp->setTextColor(WHITE, BLACK);
  
  // frame
  disp->drawFastHLine(0, 25, DISPLAY_W, WHITE);
  disp->drawFastHLine(0, 52, DISPLAY_W, WHITE);

  // Temp, Hum, Pres, Thi
  x = 0;
  y = 0;
  this->drawTemp(disp, x, y, common_data->bme_info->temp);

  x += 50;
  this->drawHum(disp, x, y, common_data->bme_info->hum);

  x -= 2;
  y += DISPLAY_CH_H * 2;
  this->drawPres(disp, x, y, common_data->bme_info->pres);

  x = DISPLAY_W - DISPLAY_CH_W * 2 * 2;
  y = 0;
  this->drawThi(disp, x, y, common_data->bme_info->thi);

  // Date/Time
  time_t t_now = time(NULL);
  struct tm *ti = localtime(&t_now);
  x = 0;
  y = 28;
  this->drawDateTime(disp, x, y, ti);

  // NTP
  x = DISPLAY_W - DISPLAY_CH_W * 3;
  y = 27;
  this->drawNtp(disp, x, y,
                common_data->ntp_info,
                common_data->netmgr_info);

  // WiFi
  x = 0;
  y = DISPLAY_H - DISPLAY_CH_H;
  this->drawWiFi(disp, x, y, common_data->netmgr_info);
} // MainMode::display()

/**
 *
 */
void MainMode::drawTemp(Display_t *disp, int x, int y, float temp) {
  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setCursor(x + 24, y + 14);
  disp->printf(".");
  disp->setCursor(x + 26, y);
  disp->printf("%cC", (char)247);

  if ( isnan(temp) ) {
    return;
  }

  char buf[5];
  sprintf(buf, "%4.1f", temp);
  buf[2] = (char)0;

  disp->setFont(&FreeSansBold12pt7b);
  disp->setTextSize(1);
  disp->setCursor(x - 1, y + 20);
  disp->printf("%s", buf);

  disp->setFont(&FreeSansBold9pt7b);
  disp->setTextSize(1);
  disp->setCursor(x + 29, y + 20);
  disp->printf("%c", buf[3]);
  disp->setFont(NULL);
} // MainMode::drawTemp()

/**
 *
 */
void MainMode::drawHum(Display_t *disp, int x, int y, float hum) {
  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setCursor(x + 23, y + 6);
  disp->printf("%%");  

  if ( isnan(hum) ) {
    return;
  }

  disp->setFont(&FreeSansBold9pt7b);
  disp->setTextSize(1);
  int h = 12;
  disp->setCursor(x, y + h);
  disp->printf("%2.0f", hum);
  disp->setFont(NULL);
} // MainMode::drawHum()

/**
 *
 */
void MainMode::drawPres(Display_t *disp, int x, int y, float pres) {
  disp->setCursor(x + DISPLAY_CH_W * 4 + 2, y);
  disp->setTextSize(1);
  disp->printf("hPa");

  if ( isnan(pres) ) {
    return;
  }

  disp->setCursor(x, y);
  disp->printf("%4.0f", pres);
} // MainMode::drawPres()

/**
 *
 */
void MainMode::drawThi(Display_t *disp, int x, int y, float thi) {
  disp->setFont(NULL);
  disp->setCursor(x + 1, y);
  disp->setTextSize(1);
  disp->printf("THI");

  if ( isnan(thi) ) {
    return;
  }

  disp->setFont(&FreeSerifBold9pt7b);
  disp->setTextSize(1);
  int h = 12;
  disp->setCursor(x, y + DISPLAY_CH_H +  h + 1);
  disp->printf("%2.0f", thi);

  disp->setFont(NULL);
} // MainMode::drawThi()

/**
 *
 */
void MainMode::drawWiFi(Display_t *disp, int x, int y, NetMgrInfo_t *ni) {
  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setCursor(x, y);

  int interval, ms;
  switch ( ni->mode ) {
  case NETMGR_MODE_START:
    if ( millis() % 500 < 500 * 70 / 100 ) {
      disp->printf("Starting WiFi");
    }
    break;

  case NETMGR_MODE_TRY_WIFI:
    if ( millis() % 500 < 500 * 70 / 100 ) {
      disp->printf("%s", ni->ssid.c_str());
    }
    break;

  case NETMGR_MODE_WIFI_ON:
    disp->printf("%s", ni->ssid.c_str());
    disp->setFont(&Picopixel);
    disp->printf(" %s", ni->ip_addr.toString().c_str());
    disp->setFont(NULL);
    break;

  case NETMGR_MODE_AP_INIT:
  case NETMGR_MODE_AP_LOOP:
    if ( millis() % 3000 < 3000 * 95 / 100 ) {
      disp->printf("[ %s ]", ni->ap_ssid.c_str());
    }
    break;

  case NETMGR_MODE_WIFI_OFF:
    if ( millis() % 500 < 500 * 50 / 100 ) {
      disp->printf("%s", ni->ssid.c_str());
    }
    break;
  } // switch(ni->mode)
} // MainMode::drawWiFi()

/**
 *
 */
void MainMode::drawDateTime(Display_t *disp, int x, int y, struct tm *ti) {
  int w = 9;
  int h = 12;
  int mon_x = x;
  int mon_y = y;
  int hour_x = mon_x + w * 4 + 12;
  int hour_y = mon_y;
  int sec_x = hour_x - 12;
  int sec_y = y + h + 4;

  int x1 = mon_x + w * 2;
  int y1 = mon_y + h - 2;
  int x2 = x1 + 3;
  int y2 = y1 - h + 4;
  disp->drawLine(x1, y1, x2, y2, WHITE);

  disp->setCursor(hour_x + w * 2, hour_y);
  disp->setFont(NULL);
  disp->setTextSize(2);
  if ( millis() % 1000 < 500 ) {
    disp->printf(":");
  }

  disp->drawRect(sec_x, sec_y, 61, 5, WHITE);
  for (int x1=sec_x; x1 <= sec_x+60; x1 += 10) {
    disp->drawFastVLine(x1, sec_y, 3, WHITE);
  }
  
  if ( ti->tm_year + 1900 < 2000 ) {
    return;
  }

  char wday_str[4];
  strftime(wday_str, sizeof(wday_str), "%a", ti);
  
  x = mon_x;
  y = mon_y;
  disp->setFont(&FreeSans9pt7b);
  disp->setTextSize(1);
  disp->setCursor(mon_x, mon_y + h);
  disp->printf("%2d\n", ti->tm_mon + 1);

  x += w * 2 + 4;
  disp->setCursor(x, y + h);
  disp->printf("%-2d", ti->tm_mday);

  x -= 8;
  y += h + 3;
  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setCursor(x, y);
  disp->printf("%s", wday_str);

  x = hour_x;
  y = hour_y;
  disp->setFont(&FreeSansBold9pt7b);
  disp->setTextSize(1);
  disp->setCursor(x, y + h);
  disp->printf("%02d", ti->tm_hour);

  disp->setCursor(x + w * 2 + 7, y + h);
  disp->printf("%02d", ti->tm_min);

  disp->fillRect(sec_x+1, sec_y, ti->tm_sec, 4, WHITE);
} // MainMode::drawDateTime()

/**
 *
 */
void MainMode::drawNtp(Display_t *disp, int x, int y,
                       NtpTaskInfo_t *ntp_info,
                       NetMgrInfo_t *netmgr_info) {
  unsigned long interval_ms = 500;
  unsigned long on_rate = 50;

  switch ( ntp_info->sntp_stat ) {
  case SNTP_SYNC_STATUS_RESET:
    interval_ms = 500;
    on_rate = 50;
    break;
  case SNTP_SYNC_STATUS_IN_PROGRESS:
    interval_ms = 2000;
    on_rate = 80;
    break;
  case SNTP_SYNC_STATUS_COMPLETED:
    interval_ms = 1000;
    on_rate = 100;
    break;
  default:
    break;
  } // switch (sntp_stat)
  if ( netmgr_info->mode != NETMGR_MODE_WIFI_ON ) {
    interval_ms = 2000;
    on_rate = 10;
  }

  //  x = DISPLAY_W - DISPLAY_CH_W * 3;
  //  y = 27;
  disp->setFont(NULL);
  disp->setTextSize(1);
  disp->setCursor(x, y);
  if ( millis() % interval_ms < interval_ms * on_rate / 100 ) {
    disp->printf("NTP");
  }

} // MainMode::drawNtp()  
