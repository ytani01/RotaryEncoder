/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _CONF_SSID_H_
#define _CONF_SSID_H_

#include <vector>
#include "ConfBase.h"

/**
 *
 */
class ConfSsid: public ConfBase {
 public:
  static constexpr char *FILE_NAME = (char *)"/ssid";

  std::vector<String> ssid;
  std::vector<String> pw;

  ConfSsid();
  virtual int load();
  virtual int save();
}; // class ConfSsid

#endif // _CONF_SSID_H_
