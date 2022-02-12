/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _CONF_BASE_H_
#define _CONF_BASE_H_

#include "esp_spiffs.h"
#include "SPIFFS.h"

/**
 *
 */
class ConfBase {
 public:
  String file_name;

  ConfBase(String file_name) {
    this->file_name = file_name;
  };

  virtual int load() {
    if ( this->open_read() < 0 ) {
      return -1;
    }

    int l_count = 0;
    while ( int fsize = this->file.available() ) {
      String line = this->read_line();
      log_i("%d: %d: \"%s\"", this->line_count, fsize, line.c_str());
    }
    this->file.close();

    return this->line_count;
  };
  
  virtual int save() {
    if ( this->open_write() < 0 ) {
      return -1;
    }

    this->write_line("test1");
    this->write_line("test2");
    this->write_line("test3");
    this->file.close();

    delay(100);
    return this->line_count;
  };
  
 protected:
  File file;
  int line_count;

  int open_read() {
    log_i("%s", this->file_name.c_str());

    if ( ! SPIFFS.begin(true) ) {
        log_e("%s: SPIFFS mount failed", this->file_name.c_str());
    }
    this->file = SPIFFS.open(this->file_name.c_str(), "r");
    if ( ! this->file ) {
      log_e("%s: open failed", file_name.c_str());
      return -1;
    }
    this->line_count = 0;
    return this->file.available();
  };

  int open_write() {
    log_i("%s", this->file_name.c_str());

    if ( ! SPIFFS.begin(true) ) {
        log_e("%s: SPIFFS mount failed", this->file_name.c_str());
    }
    this->file = SPIFFS.open(this->file_name.c_str(), "w");
    if ( ! this->file ) {
      log_e("%s: open failed", file_name.c_str());
      return -1;
    }
    this->line_count = 0;
    return this->file.available();
  };

  String read_line() {
    if ( ! this->file.available() ) {
      return "";
    }

    String line = this->file.readStringUntil('\n');
    line.trim();
    line_count++;
    return line;
  };

  String write_line(String line) {
    if ( ! this->file.available() ) {
      return "";
    }

    line.trim();
    this->file.println(line);
    this->line_count++;
    return line;
  };
}; // class ConfBase

#endif // _CONF_BASE_H_
