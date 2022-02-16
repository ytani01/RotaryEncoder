/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_NTP_TASK_H_
#define _ESP32_NTP_TASK_H_

#include <esp_sntp.h>
#include "Esp32Task.h"
#include "Esp32NetMgrTask.h"

static const char *SNTP_SYNC_STATUS_STR[] = {"RESET", "COMPLETED", "IN_PROGRESS"};

typedef struct {
  sntp_sync_status_t sntp_stat;
} Esp32NtpTaskInfo_t;

/**
 *
 */
class Esp32NtpTask: public Esp32Task {
public:
  const unsigned long INTERVAL_NORMAL = 1 * 60 * 1000; // ms
  const unsigned long INTERVAL_PROGRESS = 10 * 1000; // ms
  const unsigned long INTERVAL_NO_WIFI = 5 * 1000; // ms
  
  String *ntp_svr;
  Esp32NetMgrTask **pNetMgrTask = NULL;

  Esp32NtpTaskInfo_t info;
  
  // static function
  static char* get_time_str();

  // constructor
  Esp32NtpTask(String ntp_svr[], Esp32NetMgrTask **pNetMgrTask,
               void (*cb)(Esp32NtpTaskInfo_t *ntp_info)=NULL);

  void *get_info();

protected:
  virtual void setup();
  virtual void loop();

  void (*_cb)(Esp32NtpTaskInfo_t *ntp_info);
}; // class Esp32NtpTask
#endif // _ESP32_NTP_TASK_H_
