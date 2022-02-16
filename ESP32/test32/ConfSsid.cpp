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
    return -1;
  }

  this->ssid.clear();
  this->ssid.shrink_to_fit();
  this->pw.clear();
  this->pw.shrink_to_fit();

  int i=0;
  while ( this->file.available() ) {
    this->ssid.push_back(this->read_line());
    this->pw.push_back(this->read_line());
    log_i("|%s|%s|%d",
          this->ssid[i].c_str(), this->pw[i].c_str(), this->ssid.size());
    i++;
  }
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

  for (int i=0; i < this->ssid.size(); i++) {
    this->write_line(this->ssid[i]);
    this->write_line(this->pw[i]);
    log_i("%d|%s|%s|", i, this->ssid[i], this->pw[i]);
  }
  this->close();

  return this->line_count;
} // ConfSsid::save()
