/**
 * Copyright (c) 2022 Yoichi Tanibayaishi
 */
#include "ScanSsidMode.h"

static int count = 0;

/** constructor
 *
 */
ScanSsidMode::ScanSsidMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

  this->ssidMenu = new OledMenu("SSIDs");
  
} // ScanSsidMode::ScanssidMode()

/**
 *
 */
bool ScanSsidMode::enter(Mode_t prev_mode) {
  log_i("%s", this->get_name().c_str());

  count = 0; // static variable
  this->phase = SCANSSID_PHASE_WAITING;

  return true;
} // ScanSsidMode::enter()

/**
 *
 */
bool ScanSsidMode::exit() {
  log_i("%s", this->get_name().c_str());

  // clear scan results
  WiFi.scanDelete();
  this->phase = SCANSSID_PHASE_WAITING; // XXX
  
  // clear objects
  for (int i=0; i < this->ment.size(); i++) {
    delete this->ment[i];
  }
  this->ment.clear();
  this->ment.shrink_to_fit();
  
  this->ssidMenu->clean();

  return true;
} // ScanSsidMode::exit()

/**
 *
 */
Mode_t ScanSsidMode::reBtn_cb(ButtonInfo_t *bi) {
  if ( bi->click_count > 1 ) {
    return MODE_MENU;
  }
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  /*
   * click_count == 1
   */
  if ( this->ssidMenu->ent.size() == 0 ) {
    return MODE_MENU;
  }
  
  OledMenuDst_t dst = this->ssidMenu->select();
  log_i("dst.obj.text=\"%s\"", dst.obj.text);

  common_data->netmgr_info->new_ssid = String(dst.obj.text);
  log_i("common_data->netmgr_info->new_ssid=%s",
        common_data->netmgr_info->new_ssid.c_str());
  
  return MODE_SET_SSID;
} // ScanSsidMode::reBtn_cb()

/**
 *
 */
Mode_t ScanSsidMode::re_cb(RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle > 0 ) {
    this->ssidMenu->cursor_up();
    return MODE_N;
  }
  if ( ri->d_angle < 0 ) {
    this->ssidMenu->cursor_down();
    return MODE_N;
  }
  return MODE_N;
} // ScanSsidMode::re_cb()

/**
 *
 */
void ScanSsidMode::display(Display_t *disp) {
  int16_t scan_n;

  switch ( this->phase ) {
  case SCANSSID_PHASE_WAITING:
    if ( common_data->netmgr_info->mode == NETMGR_MODE_START
         || common_data->netmgr_info->mode == NETMGR_MODE_TRY_WIFI ) {
      /*
       * NetMgr待ち
       */
      disp->setFont(&FreeSans12pt7b);
      disp->setTextSize(1);
      disp->setCursor(10, 34);
      disp->printf("Waiting");

      disp->fillRect(0, 43, count, 4, WHITE);
      count = (count + 1) % 128;

      delay(150);
      return;
    }
    /*
     * スキャン開始
     */
    WiFi.scanDelete(); // XXX
    WiFi.scanNetworks(true, true);
    this->phase = SCANSSID_PHASE_SCANNING;
    break;

  case SCANSSID_PHASE_SCANNING:
    /*
     * スキャン終了待ち
     */
    scan_n = WiFi.scanComplete();
    if ( scan_n < 0 ) {
      disp->setFont(&FreeSans12pt7b);
      disp->setTextSize(1);
      disp->setCursor(10, 34);
      disp->printf("Scanning");

      disp->fillRect(0, 43, count, 4, WHITE);
      count = (count + 1) % 128;

      delay(150);
      return;
    }

    /*
     * SSID選択メニュー作成
     */
    for (int i=0; i < scan_n; i++) {
      if ( WiFi.SSID(i).length() == 0 ) {
        log_i("SSID \"%s\": ignored", WiFi.SSID(i).c_str());
        continue;
      }
      String ent_title = " " + WiFi.SSID(i) + " ";
      String ent_text = WiFi.SSID(i);
        
      this->ment.push_back(new OledMenuEnt(ent_title, ent_text.c_str()));
      this->ssidMenu->addEnt(this->ment.back());
    } // for(i)
    this->phase = SCANSSID_PHASE_SETTING;
    break;

  default:
    /*
     * SSID選択メニュー表示
     */
    this->ssidMenu->display(disp);
    break;
  } // switch (phase)
} // ScanSsidMode::display()
