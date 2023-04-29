/**
 * Copyright (c) 2023 Yoichi Tanibayahsi
 */
#include "Task_ScanSsid.h"

/**
 *
 */
Task_ScanSsid::Task_ScanSsid(String name)
  :Task(name) {

  for (int i=0; i < SSID_N_MAX; i++) {
    this->ssidEnt[i] = new SSIDent();
  } // for(I)
} // TaskScanSsid::TaskScanSsid()

/**
 *
 */
int16_t Task_ScanSsid::scan() {
  // WiFi status 確認
  wl_status_t wl_stat = WiFi.status();
  log_i("wl_stat=%d", wl_stat);
  if ( wl_stat == WL_NO_SHIELD ) {
    return 0;
  }

  // WiFiモード確認
  wifi_mode_t wmode = WiFi.getMode();
  log_i("wmode=%d", wmode);

  // スキャン状態確認
  int16_t res;
  while ( (res = WiFi.scanComplete()) == WIFI_SCAN_RUNNING ) {
    log_i("waing.. res=%d", res);
    task_delay(1000);
  }
  log_d("res=%d", res);

  // 前回のスキャン結果削除
  if ( this->ssidN > 0 ) {
    log_d("ssidN=%d ... scanDelete()", this->ssidN);
    WiFi.scanDelete();
    task_delay(500);
  }

  // スキャン開始
  log_i("scanNetworks...");
  WiFi.scanNetworks(true); // scan async

  // スキャン結果待ち
  while ( (this->ssidN = WiFi.scanComplete()) == WIFI_SCAN_RUNNING ) {
    log_d("ssidN=%d", this->ssidN);
    task_delay(5000); // > 500 .. 短すぎると ssidN=0 になる(!?)
  }
  log_i("ssidN=%d", this->ssidN);

  if ( this->ssidN > SSID_N_MAX ) {
    this->ssidN = SSID_N_MAX;
    log_i("ssidN=%d (SSID_N_MAX)", this->ssidN);
  }

  // スキャン結果登録
  for (int i=0; i < this->ssidN; i++) {
    this->ssidEnt[i]->set(WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i));
    log_i("%3d: %s", i, this->ssidEnt[i]->toString(true, true, true).c_str());
  } // for(i)

  return this->ssidN;
} // Task_ScanSsid::scan()

/**
 *
 */
void Task_ScanSsid::clear() {
  for (int i; i < SSID_N_MAX; i++) {
    this->ssidEnt[i]->clear();
  }
  this->ssidN = 0;
} // TaskScanSsid::clear()

/**
 *
 */
void Task_ScanSsid::setup() {
  log_i("");

  this->clear();
} // Task_ScanSsid::setup()

/**
 *
 */
void Task_ScanSsid::loop() {
  static unsigned long prev_ms = millis();
  unsigned long cur_ms = millis();
  unsigned long d_ms = cur_ms - prev_ms;

  log_i("'%s':%u:%u", this->conf.name, cur_ms, d_ms);

  this->ssidN = this->scan();

  prev_ms = cur_ms;
  if ( this->ssidN > 0 ) {
    task_delay(INTERVAL);
  } else {
    task_delay(INTERVAL_SCANNING);
  }
} // Task_ScanSsid::loop()
