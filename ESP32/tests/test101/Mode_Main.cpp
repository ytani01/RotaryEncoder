/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include "Mode_Main.h"

/**
 *
 */
Mode_Main::Mode_Main(String name, Display_t* disp)
  : Mode(name, disp) {

} // Mode_Main::Mode_Main()

/**
 *
 */
void Mode_Main::setup() {
  Mode::setup();
} // Mode_Main::enter()

/**
 *
 */
void Mode_Main::enter() {
  Mode::enter();
} // Mode_Main::enter()

/**
 *
 */
void Mode_Main::loop(unsigned long cur_ms) {
  //Disp->clearDisplay();
  Disp->setFont(NULL);
  Disp->setTextSize(1);
  Disp->setTextWrap(true);
  Disp->setTextColor(WHITE, BLACK);

  bool flag_update = false;

  // MAC addr
  char mac_str[13];
  Disp->setCursor(0, 0);
  Disp->printf("MAC:%s \n", get_mac_addr_str(mac_str));

  // WiFi stat
  static NetMgrMode_t prev_netmgr_mode = NETMGR_MODE_NULL;
  NetMgrMode_t netmgr_mode = commonData.net_mgr->cur_mode;

  static String prev_ip_str = "";
  String ip_str = commonData.net_mgr->ip_addr.toString();

  if ( netmgr_mode != prev_netmgr_mode ) {
    log_i("%s:%s", NETMGR_MODE_STR[netmgr_mode], ip_str.c_str());

    Disp->setCursor(0, 1 * (DISPLAY_CH_H + 1));
    Disp->printf("WiFi:%-16s\n %-16s \n",
                 NETMGR_MODE_STR[netmgr_mode], ip_str.c_str());

    flag_update = true;

    prev_netmgr_mode = netmgr_mode;
  }

  // NTP stat
  static sntp_sync_status_t prev_sntp_stat = SNTP_SYNC_STATUS_COMPLETED;
  sntp_sync_status_t sntp_stat = commonData.ntp->info.sntp_stat;

  if ( sntp_stat != prev_sntp_stat ) {
    log_i("NTP: %-16s", SNTP_SYNC_STATUS_STR[sntp_stat]);
    
    Disp->setCursor(0, 3 * (DISPLAY_CH_H + 1));
    Disp->printf("NTP:%-15s\n", SNTP_SYNC_STATUS_STR[sntp_stat]);

    flag_update = true;

    prev_sntp_stat = sntp_stat;
  }

  // get local time
  struct tm ti;
  getLocalTime(&ti);

  if ( ti.tm_year + 1900 > 2000 ) {

    // Date
    static String prev_date_String = "";
    String date_String = Task_Ntp::get_date_str(&ti);

    if ( date_String != prev_date_String ) {
      log_i("date: %s", date_String.c_str());

      Disp->setCursor(0, 4 * (DISPLAY_CH_H + 1));
      Disp->printf(" %-20s\n", date_String.c_str());

      flag_update = true;

      prev_date_String = date_String;
    }

    // Time
    static String prev_time_String = "";
    String time_String = Task_Ntp::get_time_str(&ti);

    if ( time_String != prev_time_String ) {
      log_d("time: %s", time_String.c_str());

      Disp->setCursor(0, 5 * (DISPLAY_CH_H + 1));
      Disp->setTextSize(2);
      Disp->printf(" %-20s\n", time_String.c_str());

      flag_update = true;

      prev_time_String = time_String;
    }

  }       

  if ( flag_update ) {
    Disp->display();
  }

  prev_ip_str = ip_str;

  task_delay(100);
} // Mode_Main::enter()

/**
 *
 */
void Mode_Main::exit() {
  Mode::exit();
} // Mode_Main::enter()
