/**
 * Copyright (c) 2021 Yoichi Tanibayashi
 */
#include "Esp32NetMgr.h"

/**
 * Initialize static variables
 */
String Esp32NetMgr::myName = "Esp32NetMgr";
unsigned int Esp32NetMgr::ssidN = 0;
SSIDent Esp32NetMgr::ssidEnt[Esp32NetMgr::SSID_N_MAX];
WebServer Esp32NetMgr::web_svr(WEBSVR_PORT);

/**
 *
 */
Esp32NetMgr::Esp32NetMgr(String ap_ssid_hdr, unsigned int try_count_max) {
  if ( ap_ssid_hdr.length() > 0 ) {
    this->ap_ssid_hdr = ap_ssid_hdr;
  }
  if ( try_count_max > 0 ) {
    this->try_count_max = try_count_max;
  }

  this->ap_ip = IPAddress(this->ap_ip_int[0],
                          this->ap_ip_int[1],
                          this->ap_ip_int[2],
                          this->ap_ip_int[3]);
  this->ap_netmask = IPAddress(this->ap_netmask_int[0],
                               this->ap_netmask_int[1],
                               this->ap_netmask_int[2],
                               this->ap_netmask_int[3]);

  esp_read_mac(this->mac_addr, ESP_MAC_WIFI_STA);
  char mac_str[13];
  sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
          this->mac_addr[0], this->mac_addr[1], this->mac_addr[2],
          this->mac_addr[3], this->mac_addr[4], this->mac_addr[5]);
  log_i("MacAddr=%s", mac_str);
} // Esp32NetMgr::Esp32NetMgr()

/**
 *
 */
Esp32NetMgrMode_t Esp32NetMgr::loop() {
  static Esp32NetMgrMode_t prev_mode = NETMGR_MODE_NULL;
  ConfWifi conf_data;
  static String ssid = "";
  static String ssid_pw = "";
  bool restart_flag = false;

  if ( this->cur_mode != prev_mode ) {
    log_i("cur_mode: %s(%d) ==> %s(%d)",
          ESP32_NETMGR_MODE_STR[prev_mode], prev_mode,
          ESP32_NETMGR_MODE_STR[this->cur_mode], this->cur_mode);
    prev_mode = this->cur_mode;
  }
  this->_loop_count++;

  if ( this->ext_cmd.length() > 0 ) {
    log_i("ext_cmd=%s", this->ext_cmd.c_str());
    if ( this->ext_cmd == "restart" ) {
      restart_flag = true;
    }
    this->ext_cmd = "";
  }

  wl_status_t wl_stat = WiFi.status();
  
  switch (this->cur_mode) {
  case NETMGR_MODE_NULL:
    break;
    
  case NETMGR_MODE_START:
    log_i("NETMGR_MODE_START");

    conf_data.load();
    ssid = conf_data.ssid;
    ssid_pw = conf_data.ssid_pw;
    log_i("|%s|%s|", ssid.c_str(), ssid_pw.c_str());

    WiFi.begin(ssid.c_str(), ssid_pw.c_str());
    delay(100);
    this->cur_mode = NETMGR_MODE_TRY_WIFI;
    this->_loop_count = 0;
    break;

  case NETMGR_MODE_TRY_WIFI:
    if ( restart_flag ) {
      log_i("restart_flag=%s", (restart_flag ? "true" : "false"));
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      this->cur_mode = NETMGR_MODE_START;
      delay(100);
      break;
    }

    if (wl_stat == WL_CONNECTED) {
      log_i("wl_stat=%s(%d): IPaddr=%s",
            WL_STATUS_T_STR[wl_stat], wl_stat,
            WiFi.localIP().toString().c_str());

      this->net_is_available = true;
      this->cur_mode = NETMGR_MODE_WIFI_ON;
      break;
    }

    if (this->_loop_count > this->try_count_max) {
      log_w(" WiFi faild");

      this->cur_mode = NETMGR_MODE_AP_INIT;
      break;
    }

    log_i("%s %d/%d wl_stat=%s(%d)",
          ESP32_NETMGR_MODE_STR[this->cur_mode],
          this->_loop_count, this->try_count_max,
          WL_STATUS_T_STR[wl_stat], wl_stat);

    delay(TRY_INTERVAL);
    break;

  case NETMGR_MODE_AP_INIT:
    // log_i("%s", this->ModeStr[this->cur_mode]);
    log_i("cur_mode=%s", ESP32_NETMGR_MODE_STR[this->cur_mode]);

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    this->ap_ssid = this->ap_ssid_hdr + "_" + this->get_mac_addr_String();

    WiFi.mode(WIFI_AP);
    log_i("WiFi.softAP: %s .. ", this->ap_ssid.c_str());
    delay(100);

    if ( ! WiFi.softAP(this->ap_ssid.c_str()) ) {
      log_i(" .. failed");
      WiFi.mode(WIFI_OFF);

      this->cur_mode = NETMGR_MODE_WIFI_OFF;
      break;
    }

    log_i(" .. start");
    delay(300);

    WiFi.softAPConfig(this->ap_ip, this->ap_ip, this->ap_netmask);

    this->dns_svr.setErrorReplyCode(DNSReplyCode::NoError);
    this->dns_svr.start(DNS_PORT, "*", this->ap_ip);
    log_i("DNS server[%d] started", DNS_PORT);

    Esp32NetMgr::async_scan_ssid_start();

    web_svr.enableDelay(false); // Important!!
    web_svr.on("/", Esp32NetMgr::handle_top);
    web_svr.on("/select_ssid", Esp32NetMgr::handle_select_ssid);
    web_svr.on("/save_ssid", Esp32NetMgr::handle_save_ssid);
    web_svr.on("/scan_ssid", Esp32NetMgr::handle_do_scan);
    web_svr.on("/confirm_reboot", Esp32NetMgr::handle_confirm_reboot);
    web_svr.on("/do_reboot", Esp32NetMgr::handle_do_reboot);
    web_svr.onNotFound(Esp32NetMgr::handle_top);
    web_svr.begin();
    log_i("Web server[%d] started", WEBSVR_PORT);

    this->cur_mode = NETMGR_MODE_AP_LOOP;

    break;

  case NETMGR_MODE_AP_LOOP:
    this->dns_svr.processNextRequest();
    web_svr.handleClient();

    if ( restart_flag ) {
      log_i("restart_flag=%s", (restart_flag ? "true" : "false"));
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      this->cur_mode = NETMGR_MODE_START;
      delay(100);
    }
    break;

  case NETMGR_MODE_WIFI_ON:
    if ( wl_stat != WL_CONNECTED ) {
      log_w("wl_stat=%s(%d)", WL_STATUS_T_STR[wl_stat], wl_stat);
      this->cur_mode = NETMGR_MODE_START;
      break;
    }

    if ( restart_flag ) {
      log_i("restart_flag=%s", (restart_flag ? "true" : "false"));
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      this->cur_mode = NETMGR_MODE_START;
      delay(100);
    }
    break;

  case NETMGR_MODE_WIFI_OFF:
    if (wl_stat == WL_CONNECTED) {
      log_i("wl_stat=%s(%d)", WL_STATUS_T_STR[wl_stat], wl_stat);
      this->cur_mode = NETMGR_MODE_WIFI_ON;
    }
    break;

  default:
    log_i("unknown mode ???");
    delay(1000);
    break;
  } // switch

  if ( this->cur_mode == NETMGR_MODE_WIFI_ON ) {
    this->cur_ssid = ssid;
  } else {
    this->cur_ssid = "";
  }
  this->cur_ssid = ssid;

  //delay(1);
  return this->cur_mode;
} // Esp32NetMgr::loop()

/**
 *
 */
String Esp32NetMgr::html_header(String title) {
  String html = "";
  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";

  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='Pragma' content='no-cache'>";
  html += "<meta http-equiv='Cache-Control' content='no-cache'>";
  html += "<meta http-equiv='Expires' content='0'>";
  html += "<meta http-equiv='Content-type' CONTENT='text/html; charset=utf-8'>";

  html += "<style>";
  html += ".myname {";
  html += "  font-size: xx-large;";
  html += "  text-align: center;";
  html += "  color: #FF0000;";
  html += "  background-color: #444444;";
  html += "}";

  html += ".ssid {";
  html += " font-size: x-large;";
  html += "}";

  html += "a:link, a:visited {";
  html += " background-color: #00AA00;";
  html += " color: white;";
  html += " border: none;";
  html += " text-align: center;";
  html += " text-decoration: none;";
  html += " display: inline-block;";
  html += " padding: 3px 7px;";
  html += "}";

  html += "a:hover, a:active {";
  html += " background-color: #00DD00;";
  html += "}";

  html += "input[type=button], input[type=submit], input[type=reset] {";
  html += " background-color: #0000DD;";
  html += " color: white;";
  html += " text-decoration: none;";
  html += " font-size: large;";
  html += " border: none;";
  html += " padding: 4px 8px;";
  html += "}";
  html += "</style>";

  html += "</head>";
  html += "<body style='background: linear-gradient(to right, #9FF, #5DD);'>";
  html += "<br />";
  html += "<div class='myname'>";
  html += Esp32NetMgr::myName;
  html += "</div>";
  html += "<div style='font-size:x-large; color:#00F; background-color: #DDD;'>";
  html += title;
  html += "</div>";
  html += "<hr>";
  return html;
} // Esp32NetMgr::handle_header()

String Esp32NetMgr::html_footer() {
  String html = "";
  html += "</body>";
  html += "</html>\n";
  return html;
} // Esp32NetMgr::html_footer();

void Esp32NetMgr::async_scan_ssid_start() {
  log_i("Esp32NetMgr::async_scan_ssid_start> ..");
  WiFi.scanNetworks(true);
} // Esp32NetMgr::async_scan_ssid_start()

unsigned int Esp32NetMgr::async_scan_ssid_wait(SSIDent ssid_ent[]) {
  int16_t ret;

  while ( (ret = WiFi.scanComplete()) == WIFI_SCAN_RUNNING ) {
    log_i("scanning.. ret=%d", ret);
    delay(500);
  }
  if ( ret == WIFI_SCAN_FAILED ) {
    log_e("SSID scan faild: ret=%d", ret);
    return 0;
  }

  if ( ret > Esp32NetMgr::SSID_N_MAX ) {
    ret = Esp32NetMgr::SSID_N_MAX;
  }

  for (int i=0; i < ret; i++) {
    ssid_ent[i].set(WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i));
  } // for()

  return ret;
} // Esp32NetMgr::async_scan_ssid_wait()

/**
 *
 */
unsigned int Esp32NetMgr::scan_ssid(SSIDent ssid_ent[]) {
  log_i("Esp32NetMgr::scan_ssid> scan start ..");

  int ssid_n = WiFi.scanNetworks();

  log_i("Esp32NetMgr::scan_ssid> %d SSIDs found", ssid_n);

  if (ssid_n <= 0) {
    return 0;
  }

  if ( ssid_n > Esp32NetMgr::SSID_N_MAX ) {
    ssid_n = Esp32NetMgr::SSID_N_MAX;
  }

  for (int i=0; i < ssid_n; i++) {
    ssid_ent[i].set(WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i));
  } // for()

  return ssid_n;
} // Esp32NetMgr::ssid_scan()

/**
 *
 */
void Esp32NetMgr::handle_top() {
  ConfWifi conf_data;
  String   ssid, ssid_pw;

  conf_data.load();
  ssid = conf_data.ssid;
  ssid_pw = conf_data.ssid_pw;
  
  log_i("ssid=%s, ssid_pw=%s", ssid.c_str(), ssid_pw.c_str());

  String html = Esp32NetMgr::html_header("Current settings");
  html += "<span style='font-size: large;'>";
  html += "SSID: ";
  html += "</span>";
  html += "<span style='font-size: x-large; font-weight: bold'>";
  html += ssid;
  html += "</span>";
  html += "<hr />";
  html += "<a href='/select_ssid'>Change</a>\n";
  html += "or\n";
  html += "<a href='/confirm_reboot'>OK</a>\n";
  html += Esp32NetMgr::html_footer();
  web_svr.send(200, "text/html", html);
} // Esp32NetMgr::handle_top()

void Esp32NetMgr::handle_select_ssid() {
  ConfWifi conf_data;
  String   ssid, ssid_pw;
  

  conf_data.load();
  ssid = conf_data.ssid;
  ssid_pw = conf_data.ssid_pw;
  
  Esp32NetMgr::ssidN = Esp32NetMgr::async_scan_ssid_wait(Esp32NetMgr::ssidEnt);
  log_i("Esp32NetMgr::ssidN=%s", String(Esp32NetMgr::ssidN));
  if (Esp32NetMgr::ssidN == 0) {
    log_i("Esp32NetMgr::handle_select_ssid> rescan SSID");
    Esp32NetMgr::async_scan_ssid_start();
    Esp32NetMgr::ssidN = Esp32NetMgr::async_scan_ssid_wait(Esp32NetMgr::ssidEnt);
  }

  for (int i=0; i < Esp32NetMgr::ssidN; i++) {
    log_i("[%2d]%4ddBm %s (%s)", i,
          Esp32NetMgr::ssidEnt[i].dbm(),
          Esp32NetMgr::ssidEnt[i].ssid().c_str(),
          Esp32NetMgr::ssidEnt[i].encType().c_str());
  } // for(i)

  String html = Esp32NetMgr::html_header("Please change settings and save");

  html += "<form action='/save_ssid' method='GET'>";
  html += "<div class='ssid'>";
  html += "SSID ";
  html += "<select name='ssid' id='ssid' style='font-size:large;'>";

  for(int i=0; i < Esp32NetMgr::ssidN; i++){
    html += "<option value=" + Esp32NetMgr::ssidEnt[i].ssid();
    if ( Esp32NetMgr::ssidEnt[i].ssid() == ssid ) {
      html += " selected";
    }
    html += ">";
    html += Esp32NetMgr::ssidEnt[i].ssid();
    /*
    html += " (";
    html += String(Esp32NetMgr::ssidEnt[i].dbm());
    html += ", ";
    html += Esp32NetMgr::ssidEnt[i].encType();
    html += ")";
    */
    html += "</option>\n";
  } // for(i)

  html += "<option value="">(clear)</option>\n";
  html += "</select><br />\n";

  html += "Password ";
  html += "<span style='font-size: xx-large'>";
  html += "<input type='password'";
  html += " name='passwd'";
  html += " value='" + ssid_pw + "'";
  html += " />";
  html += "</span>";
  html += "</div>\n";
  html += "<hr />\n";

  html += "<input type='submit' value='Save' />\n";
  html += "<a href='/scan_ssid'>Rescan</a>\n";
  html += "<a href='/'>Cancel</a>\n";

  html += "</form>";
  html += Esp32NetMgr::html_footer();

  web_svr.send(200, "text/html", html);
} // Esp32NetMgr::handle_select_ssid()

void Esp32NetMgr::handle_save_ssid(){
  ConfWifi conf_data;
  String ssid = web_svr.arg("ssid");
  String ssid_pw = web_svr.arg("passwd");
  
  log_i("save_ssid> |%s|%s|", ssid.c_str(), ssid_pw.c_str());

  conf_data.ssid = ssid;
  conf_data.ssid.trim();
  conf_data.ssid_pw = ssid_pw;
  conf_data.ssid_pw.trim();
  conf_data.print();
 
  conf_data.save();

  // 自動転送
  web_svr.sendHeader("Location", String("/"), true);
  web_svr.send(302, "text/plain", "");
} // Esp32NetMgr::handle_save_ssid()

/**
 *
 */
void Esp32NetMgr::handle_confirm_reboot() {
  String html = Esp32NetMgr::html_header("Reboot confirmation");
  html += "<p>Are you sure to reboot ";
  html += Esp32NetMgr::myName;
  html += " ?</p>\n";
  html += "<a href='/do_reboot'>Yes</a>";
  html += " or ";
  html += "<a href='/'>No</a>";
  html += Esp32NetMgr::html_footer();
  web_svr.send(200, "text/html", html.c_str());
} // Esp32NetMgr::handle_confirm_reboot()

/**
 *
 */
void Esp32NetMgr::handle_do_scan() {
  Esp32NetMgr::async_scan_ssid_start();

  // 自動転送
  web_svr.sendHeader("Location", String("/select_ssid"), true);
  web_svr.send(302, "text/plain", "");
} // Esp32NetMgr::handle_do_rescan()

/**
 *
 */
void Esp32NetMgr::handle_do_reboot() {
  String html = Esp32NetMgr::html_header("Rebooting ..");
  html += "Please reconnect WiFi ..";
  html += "<hr />";
  html += Esp32NetMgr::html_footer();
  web_svr.send(200, "text/html", html.c_str());

  log_i("reboot esp32 ..");
  delay(2000);
  ESP.restart();
} // Esp32NetMgr::handle_do_reboot()

/**
 *
 */
String Esp32NetMgr::get_mac_addr_String() {
  char buf[13];
  sprintf(buf, "%02X%02X%02X%02X%02X%02X",
          this->mac_addr[0], this->mac_addr[1], this->mac_addr[2],
          this->mac_addr[3], this->mac_addr[4], this->mac_addr[5]);

  return String(buf);
} // Esp32NetMgr::get_mac_addr_String()
