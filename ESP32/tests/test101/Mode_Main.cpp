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
  static NetMgrMode_t prev_netmgr_mode = NETMGR_MODE_NULL;
  NetMgrMode_t netmgr_mode = commonData.netMgr->cur_mode;

  static String prev_ip_str = "";
  String ip_str = commonData.netMgr->ip_addr.toString();

  if ( netmgr_mode != prev_netmgr_mode ) {
    log_i("%s:%s", NETMGR_MODE_STR[netmgr_mode], ip_str.c_str());

    commonData.disp->clearDisplay();
    commonData.disp->setCursor(2, 2);
    commonData.disp->printf(" %s \n %s \n",
                            NETMGR_MODE_STR[netmgr_mode],
                            ip_str.c_str());
    commonData.disp->display();

    prev_netmgr_mode = netmgr_mode;
    prev_ip_str = ip_str;
  }
  
  task_delay(2000);
} // Mode_Main::enter()

/**
 *
 */
void Mode_Main::exit() {
  Mode::exit();
} // Mode_Main::enter()
