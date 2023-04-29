/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#ifndef SSID_ENT_H
#define SSID_ENT_H

#include <WiFi.h>

/**
 *
 */
class SSIDent {
 public:
  SSIDent();
  SSIDent(String ssid, int dbm, wifi_auth_mode_t enc_type);
  
  void set(String ssid, int dbm, wifi_auth_mode_t enc_type);
  void clear();

  String ssid();
  int dbm();
  String encType();

  String toString(boolean flag_ssid = true,
                  boolean flag_dbm = true,
                  boolean flag_enctype = true);

  static String encType_String(wifi_auth_mode_t enc_type);

 private:
  String _ssid;
  int _dbm;
  wifi_auth_mode_t _enc_type;
};

#endif // SSID_ENT_H
