/**
 * Copyright (c) 2022 Yoichi Tanibayaishi
 */
#include "ScanSsidMode.h"

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

  WiFi.scanNetworks(true, true);
  return true;
} // ScanSsidMode::enter()

/**
 *
 */
bool ScanSsidMode::exit() {
  log_i("%s", this->get_name().c_str());

  // clear scan results
  WiFi.scanDelete();
  
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
Mode_t ScanSsidMode::reBtn_cb(Esp32ButtonInfo_t *bi) {
  if ( bi->click_count > 1 ) {
    return MODE_MAIN;
  }
  if ( bi->click_count == 0 ) {
    return MODE_N;
  }

  /*
   * click_count == 1
   */
  if ( this->ssidMenu->ent.size() == 0 ) {
    return MODE_N;
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
Mode_t ScanSsidMode::re_cb(Esp32RotaryEncoderInfo_t *ri) {
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
  int16_t ret = WiFi.scanComplete();

  static int count = 0;
  if ( ret < 0 ) {
    disp->setTextSize(1);
    disp->setCursor(5, 20);
    disp->setTextWrap(false);
    disp->printf("Scanning .", ret);
    for (int i=0; i < count; i++) {
      disp->printf(".");
    }
    count = (count + 1) % 4;
    delay(150);
    return;
  }

  if ( this->ssidMenu->ent.size() == 0 ) {
    for (int i=0; i < ret; i++) {
      if ( WiFi.SSID(i).length() == 0 ) {
        log_i("SSID \"%s\": ignored", WiFi.SSID(i).c_str());
        continue;
      }
      String ent_title = WiFi.SSID(i) + ":" + WiFi.RSSI(i);
      String ent_text = WiFi.SSID(i);

      this->ment.push_back(new OledMenuEnt(ent_title, ent_text.c_str()));
      this->ssidMenu->addEnt(this->ment.back());
    } // for(i)
  }

  this->ssidMenu->display(disp);
} // ScanSsidMode::display()
