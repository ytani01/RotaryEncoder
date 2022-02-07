/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 *
 * FreeRTOSのマルチタスクで使う場合、WDT(WatchDoc Timer)を伸ばしたほうがいい(??)
 * ```
 * #include <esp_task_wdt.h>
 *   :
 * void task_net_mgr() {
 *   ESP_ERROR_CHECK(esp_task_wdt_init(60, true)); // *** 60sec
 *   ESP_ERROR_CHECK(esp_task_wdt_add(NULL));      // *** 必要!?
 *   :
 *   while (true) { // main loop
 *     :
 *     ESP_ERROR_CHECK(esp_task_wdt_reset());      // *** 必要1?
 *   } // main loop
 * } // task_net_mgr()
 * ```
 */
#ifndef _ESP32_NETMGR_H_
#define _ESP32_NETMGR_H_

#include <esp_system.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "SSIDent.h"
#include "ConfWifi.h"

typedef enum {
              NETMGR_MODE_NULL,
              NETMGR_MODE_START,
              NETMGR_MODE_TRY_WIFI,
              NETMGR_MODE_AP_INIT,
              NETMGR_MODE_AP_LOOP,
              NETMGR_MODE_SCANNING_SSID,
              NETMGR_MODE_WIFI_ON,
              NETMGR_MODE_WIFI_OFF,
              NETMGR_MODE_N
} Esp32NetMgrMode_t;
const String ESP32_NETMGR_MODE_STR[]
= {"NULL", "START", "TRY_WIFI",
   "AP_INIT", "AP_LOOP", "SCANNING_SSID",
   "WIFI_ON", "WIFI_OFF"};

/**
 *
 */
class Esp32NetMgr {
public:
  static const unsigned int TRY_INTERVAL  = 500; // ms
  static const unsigned int DEF_TRY_COUNT_MAX = 15;
  unsigned int try_count_max = DEF_TRY_COUNT_MAX;
  
  static const unsigned int SSID_N_MAX = 50;
  
  static const int DNS_PORT    = 53;
  static const int WEBSVR_PORT = 80;

  // XXX static constにしたいが、初期化が面倒！
  char *ModeStr[Esp32NetMgrMode_t::NETMGR_MODE_N] = 
    {(char *)"NULL", (char *)"START", (char *)"TRY_WIFI",
     (char *)"AP_INIT", (char *)"AP_LOOP", (char *)"SCANNING",
     (char *)"WIFI_ON", (char *)"WIFI_OFF"};
  Esp32NetMgrMode_t cur_mode = NETMGR_MODE_START;
  
  static String myName;
  static unsigned int ssidN;
  static SSIDent ssidEnt[SSID_N_MAX];

  uint8_t mac_addr[6];
  String cur_ssid = "";

  boolean net_is_available = false;;

  String ap_ssid_hdr = "ESP32_AP_";
  String ap_ssid = ap_ssid_hdr;
  String ap_ssid_pw  = "xxxxxxxx"; // unused

  int ap_ip_int[4]      = {192,168,111,1};
  int ap_netmask_int[4] = {255,255,255,0};

  IPAddress ap_ip;       // initialize in constructor
  IPAddress ap_netmask;  // initialize in constructor
  DNSServer dns_svr;

  Esp32NetMgr(String ap_ssid_hdr, unsigned int try_count_max=DEF_TRY_COUNT_MAX);
  Esp32NetMgrMode_t loop();

  String get_mac_addr_String();

  void set_ssid(String ssid="");
  
protected:
  unsigned int _loop_count = 0;

  static WebServer web_svr;

  static unsigned int scan_ssid(SSIDent ssid_ent[]);
  static void         async_scan_ssid_start();
  static unsigned int async_scan_ssid_wait(SSIDent ssid_ent[]);

  static String html_header(String title);
  static String html_footer();

  static void handle_top();
  static void handle_select_ssid();
  static void handle_save_ssid();
  static void handle_do_scan();
  static void handle_confirm_reboot();
  static void handle_do_reboot();
};

#endif // _ESP32_NETMGR_H_
