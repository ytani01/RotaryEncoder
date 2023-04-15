/**
 * $Id$
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <Arduino.h>
#include <vector>
#include <esp_system.h>
#include <esp32-hal-log.h>

#include "commonlib.h"
#include "NetMgr.h"
#include "Display.h"

#define VERSION_STR "0.01"

/*
typedef struct {
  NetMgrMode_t mode;
  uint8_t mac_addr[6];
  IPAddress ip_addr;
  String new_ssid;
  String ssid;
  String ap_ssid;
} NetMgrInfo_t;
*/

typedef struct {
  NetMgr* netMgr;
  Display_t* disp;
} CommonData_t;

extern CommonData_t commonData;

#endif // _COMMON_H_
