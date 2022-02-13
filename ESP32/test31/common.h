/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
// #include <stack> // XXX
#include <esp32-hal-log.h>

#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"
#include "Esp32Bme280.h"

typedef enum {
              MODE_MAIN,
              MODE_MENU,
              MODE_RESTART,
              MODE_SET_TEMP_OFFSET,
              MODE_SCAN_SSID,
              MODE_SET_SSID,
              MODE_N
} Mode_t;
static const char *MODE_T_STR[] = {"MAIN", "MENU", "RESTART",
                                   "SET_TEMP_OFFSET",
                                   "SCAN_SSID", "SET_SSID"};

typedef struct {
  String msg;
  Mode_t cur_mode;
  // std::stack<Mode_t> mode_stack; // XXX
  Esp32NetMgrInfo_t *netmgr_info;
  Esp32NtpTaskInfo_t *ntp_info;
  Esp32Bme280Info_t *bme_info;
} CommonData_t;
#endif // _COMMON_H_
