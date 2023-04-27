/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Task_Ntp.h"

/**
 * defulat callback
 */
static void _ntp_cb(Task_NtpInfo_t *ntp_info) {
  struct tm ti;
  getLocalTime(&ti);
  
  log_i("ntp_info->sntp_stat=%s:%d, %s",
        SNTP_SYNC_STATUS_STR[ntp_info->sntp_stat],
        ntp_info->sntp_stat, Task_Ntp::get_date_time_str(&ti));
} // _ntp_cb()

/** static
 *
 */
char* Task_Ntp::get_date_str(struct tm *ti) {
  //staic const String wday_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  static char buf[4+1+2+1+2 +1+3+1 +1];

  strftime(buf, sizeof(buf), "%Y-%m-%d(%a)", ti);
  return buf;
} // Task_Ntp::get_date_str()

/** static
 *
 */
char* Task_Ntp::get_time_str(struct tm *ti) {
  static char buf[2+1+2+1+2 +1];

  strftime(buf, sizeof(buf), "%H:%M:%S", ti);
  return buf;
} // Task_Ntp::get_date_str()

/** static
 *
 */
char* Task_Ntp::get_date_time_str(struct tm *ti) {
  //const String day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
  static char buf[4+1+2+1+2 +1+3+1 +1 +2+1+2+1+2 +1];

  strftime(buf, sizeof(buf), "%Y-%m-%d(%a) %H:%M:%S", ti);
  return buf;
} // Task_Ntp::get_time_str()

/** constructor
 *
 */
Task_Ntp::Task_Ntp(String name, String ntp_svr[], NetMgr *net_mgr,
                   void (*cb)(Task_NtpInfo_t *ntp_info))
  : Task(name) {

  this->ntp_svr = ntp_svr;
  this->net_mgr = net_mgr;
  this->_cb = cb;
  if ( cb == NULL ) {
    this->_cb = _ntp_cb;
  }

  this->info.sntp_stat = SNTP_SYNC_STATUS_RESET;
} // Task_Ntp::Task_Ntp

/**
 *
 */
void* Task_Ntp::get_info() {
  return (void *)&(this->info);
} // Task_Ntp::get_info()


/**
 *
 */
void Task_Ntp::setup() {
  log_i("%s", this->conf.name);

  setenv("TZ", "JST-9", 1);
  tzset();
  //sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
} // Task_Ntp::setup()

/**
 *
 */
void Task_Ntp::loop() {
  unsigned long interval = INTERVAL_NO_WIFI;

  if ( this->net_mgr->cur_mode != NETMGR_MODE_WIFI_ON ) {
    log_w("WIFI is not available");
    this->info.sntp_stat = SNTP_SYNC_STATUS_RESET;

    this->_cb(&(this->info));
    task_delay(INTERVAL_NO_WIFI);
    return;
  }
    
  // start sync
  log_i("start sync ..");
  configTime(9 * 3600L, 0,
             ntp_svr[0].c_str(), ntp_svr[1].c_str(), ntp_svr[2].c_str());

  /*
   * sntp_get_sync_status()
   *   同期未完了の場合、SNTP_SYNC_STATUS_RESET
   *   同期が完了すると「一度だけ」、SNTP_SYNC_STATUS_COMPLETE
   *   SNTP_SYNC_MODE_SMOOTHの同期中の場合は、SNTP_SYNC_STAUS_IN_PROGRESS)
   */
  static sntp_sync_status_t prev_stat = SNTP_SYNC_STATUS_RESET;
  this->info.sntp_stat = sntp_get_sync_status();

  struct tm ti;
  getLocalTime(&ti);
  
  if ( this->info.sntp_stat == SNTP_SYNC_STATUS_COMPLETED ) {
    interval = INTERVAL_NORMAL;
    if ( prev_stat != SNTP_SYNC_STATUS_COMPLETED ) {
      log_i("%s: NTP sync done: sntp_stat=%s(%d), interval=%'d",
            get_date_time_str(&ti),
            SNTP_SYNC_STATUS_STR[this->info.sntp_stat], this->info.sntp_stat,
            interval);
    }

  } else if ( this->info.sntp_stat == SNTP_SYNC_STATUS_IN_PROGRESS ) {
    interval = INTERVAL_PROGRESS;
    log_i("%s: NTP sync progress: sntp_stat=%s:%d, interval=%'d",
          get_date_time_str(&ti),
          SNTP_SYNC_STATUS_STR[this->info.sntp_stat], this->info.sntp_stat,
          interval);

  } else {
    interval = INTERVAL_NO_WIFI;
    log_i("%s: NTP sync retry: sntp_stat=%s(%d), interval=%'d",
          get_date_time_str(&ti),
          SNTP_SYNC_STATUS_STR[this->info.sntp_stat], this->info.sntp_stat,
          interval);
  }

  if ( this->_cb ) {
    this->_cb(&(this->info));
  }

  prev_stat = this->info.sntp_stat;
  task_delay(interval);
} // Task_Ntp::loop()
