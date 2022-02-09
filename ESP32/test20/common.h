/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <Arduino.h>
#include <esp32-hal-log.h>

#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"
#include "Esp32Bme280.h"
#include "Bme280Setup.h"

typedef enum {
              MODE_MAIN,
              MODE_MENU,
              MODE_SET_TEMP_OFFSET,
              MODE_SET_WIFI,
              MODE_N
} Mode_t;
static const String MODE_T_STR[]
= {"MAIN", "MENU", "SET_TEMP_OFFSET", "SET_WIFI"};

typedef struct {
  String msg;
  Esp32NetMgrInfo_t *netmgr_info;
  Esp32NtpTaskInfo_t *ntp_info;
  Esp32Bme280Info_t *bme_info;
} CommonData_t;
#endif // _COMMON_H_
