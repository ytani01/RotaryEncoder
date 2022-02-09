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
void MainMode::display(Display_t *disp, float fps) {
  int x, y;
  
  disp->setTextWrap(false);
  disp->setTextColor(WHITE, BLACK);
  
  disp->drawFastHLine(0, 25, DISPLAY_W - 1, WHITE);
  disp->drawFastHLine(0, 53, DISPLAY_W - DISPLAY_CH_W * 5, WHITE);


  time_t t_now = time(NULL);
  struct tm *ti = localtime(&t_now);
  x = 0;
  y = 28;
  this->drawDateTime(disp, x, y, ti);

  x = DISPLAY_W - DISPLAY_CH_W * 2;
  y = 30;
  this->drawNtp(disp, x, y,
                common_data->ntp_info,
                common_data->netmgr_info);

  x = 0;
  y = DISPLAY_H - DISPLAY_CH_H;
  this->drawWiFi(disp, x, y, common_data->netmgr_info);


  x = DISPLAY_W - DISPLAY_CH_W * 4;
  y = DISPLAY_H - DISPLAY_CH_H - 4;
  disp->fillRect(x-1, y, DISPLAY_CH_W * 4, DISPLAY_CH_H, BLACK);
  disp->setCursor(x, y);
  disp->setTextSize(1);
  disp->printf("%.1f", fps);
} // MainMode::display()

/**
 *
 */
void MainMode::drawWiFi(Display_t *disp, int x, int y, Esp32NetMgrInfo_t *ni) {
  disp->setCursor(x, y);
  disp->setTextSize(1);
  if ( ni->mode == NETMGR_MODE_WIFI_ON ) {
    disp->printf("SSID:%s", ni->ssid.c_str());
  } else if ( ni->mode == NETMGR_MODE_AP_LOOP ) {
    disp->printf("AP:%s", ni->ssid.c_str());
  } else {
    disp->printf("WiFi..");
  }  
} // OledTask::drawWiFi()

/**
 *
 */
void MainMode::drawNtp(Display_t *disp, int x, int y,
                       Esp32NtpTaskInfo_t *ntp_info,
                       Esp32NetMgrInfo_t *netmgr_info) {
  String ntp_stat_str = "?";

  switch ( ntp_info->sntp_stat ) {
  case SNTP_SYNC_STATUS_RESET:
    ntp_stat_str = "!";
    break;
  case SNTP_SYNC_STATUS_IN_PROGRESS:
    ntp_stat_str = "~";
    break;
  case SNTP_SYNC_STATUS_COMPLETED:
    ntp_stat_str = "=";
    break;
  default:
    break;
  } // switch (sntp_stat)
  if ( netmgr_info->mode != NETMGR_MODE_WIFI_ON ) {
    ntp_stat_str = "x";
  }

  x = DISPLAY_W - DISPLAY_CH_W * 2;
  y = 30;
  disp->setCursor(x, y);
  disp->setTextSize(2);
  disp->printf("%s", ntp_stat_str.c_str());
} // MainMode::drawNtp()  

/**
 *
 */
void MainMode::drawDateTime(Display_t *disp, int x, int y, struct tm *ti) {
  int mon_x = x - 2;
  int mon_y = y;
  int hour_x = mon_x + DISPLAY_CH_W * 2 * 5 + 1;
  int hour_y = mon_y + 1;
  int sec_x = x + 53;
  int sec_y = y + 18;


  int x1 = mon_x + DISPLAY_CH_W * 2 * 2;
  int y1 = mon_y + DISPLAY_CH_H * 2 - 4;
  int x2 = x1 + DISPLAY_CH_W - 2;
  int y2 = y1 - DISPLAY_CH_H * 2 + 6;
  disp->drawLine(x1, y1, x2, y2, WHITE);

  if ( millis() % 1000 >= 500 ) {
    disp->setCursor(hour_x + DISPLAY_CH_W * 2 * 2 - 4, hour_y);
    disp->setTextSize(2);
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
  disp->setCursor(mon_x, mon_y);
  disp->setTextSize(2);
  disp->printf("%2d\n", ti->tm_mon + 1);

  x += DISPLAY_CH_W * (2 * 2 + 1);
  disp->setCursor(x, y);
  disp->setTextSize(2);
  disp->printf("%-2d", ti->tm_mday);

  y += DISPLAY_CH_H * 2;
  disp->setCursor(x, y);
  disp->setTextSize(1);
  disp->printf("%s", wday_str);

  x = hour_x;
  y = hour_y;
  disp->setCursor(x, y);
  disp->setTextSize(2);
  disp->printf("%02d", ti->tm_hour);

  disp->setCursor(x + DISPLAY_CH_W * 2 * 2 + 4, y);
  disp->printf("%02d", ti->tm_min);

  disp->fillRect(sec_x+1, sec_y, ti->tm_sec, 4, WHITE);
} // MainMode::drawDateTime()
