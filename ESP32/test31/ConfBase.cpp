/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "ConfBase.h"

/** constructor
 *
 */
ConfBase::ConfBase(String file_name) {
  this->file_name = file_name;

  if ( ! SPIFFS.begin(true) ) {
    log_e("%s: SPIFFS mount failed", this->file_name.c_str());
  }
} // ConfBase::ConfBase()

/** virtual
 *
 */
int ConfBase::load() {
  // test code
  if ( this->open_read() < 0 ) {
    return -1;
  }

  while ( int fsize = this->file.available() ) {
    String line = this->read_line();
    log_i("%d: %d: \"%s\"", this->line_count, fsize, line.c_str());
  }
  this->file.close();

  return this->line_count;
} // ConfBase::load()

/** virtual
 *
 */
int ConfBase::save() {
  // test code
  if ( this->open_write() < 0 ) {
    return -1;
  }

  this->write_line("test1");
  this->write_line("test2");
  this->write_line("test3");
  this->file.close();
  
  delay(100);
  return this->line_count;
} // ConfBase::save()
  
/** protected
 *
 */
int ConfBase::open_read() {
  log_i("%s", this->file_name.c_str());

  this->file = SPIFFS.open(this->file_name.c_str(), "r");
  if ( ! this->file ) {
    log_e("%s: open failed", file_name.c_str());
    return -1;
  }
  this->line_count = 0;
  return this->file.available();
} // ConfBase::open_read()

/** protected
 *
 */
int ConfBase::open_write() {
  log_i("%s", this->file_name.c_str());

  this->file = SPIFFS.open(this->file_name.c_str(), "w");
  if ( ! this->file ) {
    log_e("%s: open failed", file_name.c_str());
    return -1;
  }
  this->line_count = 0;
  return this->file.available();
} // ConfBase::open_write()

/** protected
 *
 */
void ConfBase::close() {
  this->file.close();
} // ConfBase::close()

/** protected
 *
 */
String ConfBase::read_line() {
  if ( ! this->file.available() ) {
    return "";
  }

  String line = this->file.readStringUntil('\n');
  line.trim();
  this->line_count++;
  return line;
} // ConfBase::read_line()

/** protected
 *
 */
String ConfBase::write_line(String line) {
  if ( ! this->file.available() ) {
    return "";
  }

  line.trim();
  this->file.println(line);
  this->line_count++;
  return line;
} // ConfBase::write_line()
