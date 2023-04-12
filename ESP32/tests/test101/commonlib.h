/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _COMMONLIB_H_
#define _COMMONLIB_H_

#include <Arduino.h>
#include <esp_system.h>

char*  get_mac_addr(char* mac_str);
String get_mac_addr_String();

void task_delay(uint32_t ms);

#endif // _COMMONLIB_H_
