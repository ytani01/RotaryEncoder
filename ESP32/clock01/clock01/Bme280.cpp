/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Bme280.h"

/**
 *
 */
Bme280::Bme280(uint8_t addr, float temp_offset) {
  this->_info.addr = addr;
  this->_info.temp_offset = temp_offset;

  this->_info.active = this->_bme.begin(this->_info.addr);

  log_i("addr=active=%d", this->_info.active);
} // Bme280::Bme280()

/**
 *
 */
bool Bme280::is_active() {
  return this->_info.active;
} // Bme280::is_active();

/**
 *
 */
void Bme280::set_temp_offset(float temp_offset) {
  this->_info.temp_offset = temp_offset;
} // Bme280::set_temp_offset()

/**
 *
 */
float Bme280::get_temp_offset() {
  return this->_info.temp_offset;
} // Bme280::get_temp_offset()

/**
 * @param [out] info
 */
Bme280Info_t *Bme280::get() {
  if ( ! this->_info.active ) {
    log_w("active=%d", this->_info.active);
    this->_info.temp=NAN;
    this->_info.hum=NAN;
    this->_info.pres=NAN;
    this->_info.thi=NAN;

    return &this->_info;
  }

  this->_info.temp = this->_bme.readTemperature() + this->_info.temp_offset;
  this->_info.hum = this->_bme.readHumidity();
  this->_info.pres = this->_bme.readPressure() / 100;

  this->_info.thi = this->calc_thi(this->_info.temp, this->_info.hum);

  log_d("%.1f C(%.1f) %.0f%% %0.0f hPa %.1f thi",
        this->_info.temp, this->_info.temp_offset,
        this->_info.hum, this->_info.pres, this->_info.thi);
  return &this->_info;
} // Bme280::get()

/** static function
 *
 */
float Bme280::calc_thi(float temp, float hum) {
  return  0.81 * temp + 0.01 * hum * (0.99 * temp - 14.3) + 46.3;
} // Bme280::calc_thi()
