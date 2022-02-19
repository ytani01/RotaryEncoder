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

  WiFi.scanNetworks(true, true);
  count = 0;
  
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
Mode_t ScanSsidMode::reBtn_cb(ButtonInfo_t *bi) {
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
  int16_t ret = WiFi.scanComplete();

  if ( ret < 0 ) {
    disp->setFont(&FreeSans12pt7b);
    disp->setTextSize(1);
    disp->setCursor(10, 34);
    disp->printf("Scanning");

    disp->fillRect(0, 43, count, 4, WHITE);
    count = (count + 1) % 128;

#if 0
    disp->setFont(NULL);
    disp->setTextSize(1);
    disp->setCursor(10,0);
    disp->printf("ret=%d", ret);
#endif     
    
    delay(100);
    return;
  }

  if ( this->ssidMenu->ent.size() == 0 ) {
    for (int i=0; i < ret; i++) {
      if ( WiFi.SSID(i).length() == 0 ) {
        log_i("SSID \"%s\": ignored", WiFi.SSID(i).c_str());
        continue;
      }
      String ent_title = " " + WiFi.SSID(i) + " ";
      String ent_text = WiFi.SSID(i);

      this->ment.push_back(new OledMenuEnt(ent_title, ent_text.c_str()));
      this->ssidMenu->addEnt(this->ment.back());
    } // for(i)
    return;
  }

  this->ssidMenu->display(disp);
} // ScanSsidMode::display()
