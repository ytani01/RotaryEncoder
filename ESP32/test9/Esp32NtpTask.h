/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_NTP_TASK_H_
#define _ESP32_NTP_TASK_H_

#include <esp_sntp.h>
#include "Esp32Task.h"
#include "Esp32NetMgrTask.h"

/**
 *
 */
class Esp32NtpTask: public Esp32Task {
public:
  const unsigned long INTERVAL_NORMAL = 5 * 60 * 1000; // ms
  const unsigned long INTERVAL_PROGRESS = 10 * 1000; // ms
  const unsigned long INTERVAL_NO_WIFI = 20 * 1000; // ms
  const uint32_t WDT_SEC = INTERVAL_NORMAL / 1000 * 2; // sec
  
  String *ntp_svr;
  Esp32NetMgrTask **pNetMgrTask = NULL;
  
  // static function
  static char* get_time_str();

  // constructor
  Esp32NtpTask(String ntp_svr[], Esp32NetMgrTask **pNetMgrTask);

protected:
  virtual void setup();
  virtual void loop();
}; // class Esp32NtpTask
#endif // _ESP32_NTP_TASK_H_
