/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _CONF_SSID_H_
#define _CONF_SSID_H_

#include "ConfBase.h"

/**
 *
 */
class ConfSsid: public ConfBase {
 public:
  static constexpr char *FILE_NAME = (char *)"/ssid";

  String ssid, ssid_pw;

  ConfSsid();
  virtual int load();
  virtual int save();
}; // class ConfSsid

#endif // _CONF_SSID_H_
