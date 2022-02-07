/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <Arduino.h>
#include <esp32-hal-log.h>

/**
 * Primary Mode
 */
typedef enum {
              MODE_MAIN,
              MODE_MENU,
              MODE_SET_TEMP_OFFSET,
              MODE_SET_WIFI,
              MODE_N
} Mode_t;
static const String MODE_T_STR[]
= {"MAIN", "MENU", "SET_TEMP_OFFSET", "SET_WIFI"};

#endif // _COMMON_H_
