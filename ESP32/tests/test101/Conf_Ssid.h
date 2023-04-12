/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#ifndef _CONF_SSID_H_
#define _CONF_SSID_H_

#include <unordered_map>
#include "Conf.h"

/**
 *
 */
class Conf_Ssid: public Conf {
 public:
  static constexpr char *FILE_NAME = (char *)"/ssid";

  std::unordered_map<std::string, std::string> ent;

  Conf_Ssid();
  virtual int load();
  virtual int save();
}; // class Conf_Ssid

#endif // _CONF_SSID_H_
