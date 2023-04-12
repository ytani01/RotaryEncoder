/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include "Conf.h"

/** constructor
 *
 */
Conf::Conf(String file_name) {
  this->file_name = file_name;

  if ( ! SPIFFS.begin(true) ) {
    log_e("%s: SPIFFS mount failed", this->file_name.c_str());
  }
} // Conf::Conf()

/** virtual
 *
 */
int Conf::load() {
  // test code
  if ( ! this->open_read() ) {
    return -1;
  }

  while ( int fsize = this->file.available() ) {
    String line = this->read_line();
    log_i("%d: %d: \"%s\"", this->line_count, fsize, line.c_str());
  }
  this->file.close();

  return this->line_count;
} // Conf::load()

/** virtual
 *
 */
int Conf::save() {
  // test code
  if ( ! this->open_write() ) {
    return -1;
  }

  this->write_line("test1");
  this->write_line("test2");
  this->write_line("test3");
  this->file.close();
  
  return this->line_count;
} // Conf::save()
  
/** protected
 *
 */
bool Conf::open_read() {
  log_i("%s", this->file_name.c_str());

  this->file = SPIFFS.open(this->file_name, "r");
  if ( ! this->file ) {
    log_e("%s: open failed", file_name.c_str());
    return false;
  }
  this->line_count = 0;
  return true;
} // Conf::open_read()

/** protected
 *
 */
bool Conf::open_write() {
  log_i("%s", this->file_name.c_str());

  this->file = SPIFFS.open(this->file_name, "w");
  if ( ! this->file ) {
    log_e("%s: open failed", file_name.c_str());
    return false;
  }
  this->line_count = 0;
  return true;
} // Conf::open_write()

/** protected
 *
 */
void Conf::close() {
  this->file.close();
} // Conf::close()

/** protected
 *
 */
String Conf::read_line() {
  int ret = this->file.available();
  if ( ret <= 0 ) {
    log_w("ret=%d: EOF?", ret);
    return "";
  }

  String line = this->file.readStringUntil('\n');
  line.trim(); // XXX Important !!
  this->line_count++;
  log_i("%5d|%s|", line_count, line.c_str());
  return line;
} // Conf::read_line()

/** protected
 *
 */
String Conf::write_line(String line) {
  line.trim(); // XXX Important !!
  this->file.println(line);
  this->line_count++;
  log_i("%5d|%s|", line_count, line.c_str());
  return line;
} // Conf::write_line()
