/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _TASK_NTP_H_
#define _TASK_NTP_H_

#include <esp_sntp.h>
#include "Task.h"
#include "Task_NetMgr.h"

static const char *SNTP_SYNC_STATUS_STR[] = {"RESET", "COMPLETED", "IN_PROGRESS"};

typedef struct {
  sntp_sync_status_t sntp_stat;
} Task_NtpInfo_t;

/**
 *
 */
class Task_Ntp: public Task {
public:
  const unsigned long INTERVAL_NORMAL = 1 * 60 * 1000; // ms
  const unsigned long INTERVAL_PROGRESS = 10 * 1000; // ms
  const unsigned long INTERVAL_NO_WIFI = 5 * 1000; // ms

  String *ntp_svr;
  NetMgr *net_mgr = NULL;

  Task_NtpInfo_t info;
  
  Task_Ntp(String name, String ntp_svr[], NetMgr* net_mgr,
           void (*cb)(Task_NtpInfo_t *ntp_info)=NULL);

  void *get_info();

  // static function
  static char* get_date_str(struct tm *ti);
  static char* get_time_str(struct tm *ti);
  static char* get_date_time_str(struct tm *ti);

protected:
  virtual void setup();
  virtual void loop();

  void (*_cb)(Task_NtpInfo_t *ntp_info);
}; // class Task_Ntp
#endif // _TASK_NTP_H_
