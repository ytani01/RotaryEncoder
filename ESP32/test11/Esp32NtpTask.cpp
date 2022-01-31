/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32NtpTask.h"

/**
 *
 */
char* Esp32NtpTask::get_time_str() {
  struct tm ti; // time info
  const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  static char buf[4+1+2+1+2 +1+3+1 +1 +2+1+2+1+2 +1];

  getLocalTime(&ti);
  strftime(buf, sizeof(buf), "%Y-%m-%d(%a) %H:%M:%S", &ti);
  return buf;
} // Esp32NetMgrTask::get_time_str()

/**
 *
 */
Esp32NtpTask::Esp32NtpTask(String ntp_svr[], Esp32NetMgrTask **pNetMgrTask)
  : Esp32Task("NTP_task") {

  this->ntp_svr = ntp_svr;
  this->pNetMgrTask = pNetMgrTask;
} // Esp32NtpTask::Esp32NtpTask

/**
 *
 */
void Esp32NtpTask::setup() {
  log_d("%s", this->conf.name);

  setenv("TZ", "JST-9", 1);
  tzset();
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
} // Esp32NtpTask::setup()

/**
 *
 */
void Esp32NtpTask::loop() {
  bool wifi_available = false;
  unsigned long interval = INTERVAL_NO_WIFI;

  Esp32NetMgrTask *netMgrTask = *(this->pNetMgrTask);
  if ( netMgrTask != NULL ) {
    Esp32NetMgr *netMgr = netMgrTask->netMgr;
    if ( netMgr != NULL ) {
      Esp32NetMgrMode_t netMgrMode = netMgr->cur_mode;
      if ( netMgrMode == NETMGR_MODE_WIFI_ON ) {
        wifi_available = true;
      }
    }
  }

  if ( ! wifi_available ) {
    log_w("WIFI is not available");
    delay(INTERVAL_NO_WIFI);
    return;
  }
    
  // start sync
  log_d("start sync ..");
  configTime(9 * 3600L, 0,
             ntp_svr[0].c_str(), ntp_svr[1].c_str(), ntp_svr[2].c_str());

  /*
   * sntp_get_sync_status()
   *   同期未完了の場合、SNTP_SYNC_STATUS_RESET
   *   動機が完了すると「一度だけ」、SNTP_SYNC_STATUS_COMPLETE
   *   SNTP_SYNC_MODE_SMOOTHの同期中の場合は、SNTP_SYNC_STAUS_IN_PROGRESS)
   */
  sntp_sync_status_t sntp_stat = sntp_get_sync_status();
  if ( sntp_stat == SNTP_SYNC_STATUS_COMPLETED ) {
    interval = INTERVAL_NORMAL;
    log_i("%s: NTP sync done: sntp_stat=%d, interval=%'d",
          get_time_str(), sntp_stat, interval);
  } else if ( sntp_stat == SNTP_SYNC_STATUS_IN_PROGRESS ) {
    interval = INTERVAL_PROGRESS;
    log_i("%s: NTP sync progress: sntp_stat=%d, interval=%'d",
          get_time_str(), sntp_stat, interval);
  } else {
    interval = INTERVAL_NO_WIFI;
    log_i("%s: NTP sync retry: sntp_stat=%d, interval=%'d",
          get_time_str(), sntp_stat, interval);
  }

  delay(interval);
} // Esp32NtpTask::loop()
