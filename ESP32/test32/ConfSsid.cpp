/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "ConfSsid.h"

/** constructor
 *
 */
ConfSsid::ConfSsid()
  : ConfBase(String(ConfSsid::FILE_NAME)) {
} // ConfSsid::ConfSsid()

/** virtual
 *
 */
int ConfSsid::load() {
  if ( this->open_read() < 0 ) {
    this->ssid = "";
    this->ssid_pw = "";
    return -1;
  }

  this->ssid = this->read_line();
  this->ssid_pw = this->read_line();
  this->close();

  return this->line_count;
} // ConfSsid::load()

/** virtual
 *
 */
int ConfSsid::save() {
  if ( this->open_write() < 0 ) {
    return -1;
  }

  this->write_line(this->ssid);
  this->write_line(this->ssid_pw);
  this->close();

  return this->line_count;
} // ConfSsid::save()
