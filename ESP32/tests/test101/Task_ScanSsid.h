/**
 * Copyright (c) 2023 Yoichi Tanibayahsi
 */
#ifndef _TASK_SCAN_SSID_H_
#define _TASK_SCAN_SSID_H_

#include "Task.h"

//#include <esp_wifi.h>
#include <WiFi.h>
#include "SSIDent.h"

/**
 *
 */
class Task_ScanSsid: public Task {

 public:
  static const uint16_t  SSID_N_MAX = 20;
  static const unsigned long INTERVAL = 20 * 1000;
  static const unsigned long INTERVAL_SCANNING = 1 * 1000;

  SSIDent* ssidEnt[SSID_N_MAX];
  int16_t ssidN = 0;

  Task_ScanSsid(String name);

  int16_t scan();
  void clear();

 protected:
  virtual void setup();
  virtual void loop();
  
}; // class Task_ScanSsid
#endif // _TASK_SCAN_SSID_H_
