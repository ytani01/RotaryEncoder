/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "ConfTemp.h"

/** constructor
 *
 */
ConfTemp::ConfTemp()
  : ConfBase(String(ConfTemp::FILE_NAME)) {
} // ConfTemp::ConfTemp()

/** virtual
 *
 */
int ConfTemp::load() {
  if ( this->open_read() < 0 ) {
    this->temp_offset = 0.0;
    return -1;
  }

  String line = this->read_line();
  log_i("line=%s", line.c_str());

  this->temp_offset = line.toFloat();
  this->close();
  
  return this->line_count;
} // ConfTemp::load()

/** virtual
 *
 */
int ConfTemp::save() {
  int ret = this->open_write();
  log_i("ret=%d", ret);
  if ( ret < 0 ) {
    return -1;
  }

  String line = String(this->temp_offset);
  log_i("line=\"%s\"", line);
  this->write_line(line);
  delay(100);
  this->close();

  return this->line_count;
} // ConfTemp::save()
