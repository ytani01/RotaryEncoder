/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32Bme280.h"

/**
 *
 */
Esp32Bme280::Esp32Bme280(uint8_t addr, float temp_offset) {
  this->_info.addr = addr;
  this->_info.temp_offset = temp_offset;

  this->_info.active = this->_bme.begin(this->_info.addr);

  log_i("active=%d", this->_info.active);
} // Esp32Bme280::Esp32Bme280()

/**
 *
 */
bool Esp32Bme280::is_active() {
  return this->_info.active;
} // Esp32Bme280::is_active();

/**
 *
 */
void Esp32Bme280::set_temp_offset(float temp_offset) {
  this->_info.temp_offset = temp_offset;
} // Esp32Bme280::set_temp_offset()

/**
 *
 */
float Esp32Bme280::get_temp_offset() {
  return this->_info.temp_offset;
} // Esp32Bme280::get_temp_offset()

/**
 * @param [out] info
 */
Esp32Bme280Info_t *Esp32Bme280::get() {
  if ( ! this->_info.active ) {
    log_w("active=%d", this->_info.active);
    return NULL;
  }

  this->_info.temp = this->_bme.readTemperature() + this->_info.temp_offset;
  this->_info.hum = this->_bme.readHumidity();
  this->_info.pres = this->_bme.readPressure() / 100;

  this->_info.thi = this->calc_thi(this->_info.temp, this->_info.hum);

  log_i("%.1f C(%.1f) %.0f%% %0.0f hPa %.1f thi",
        this->_info.temp, this->_info.temp_offset,
        this->_info.hum, this->_info.pres, this->_info.thi);
  return &this->_info;
} // Esp32Bme280::get()

/** static function
 *
 */
float Esp32Bme280::calc_thi(float temp, float hum) {
  return  0.81 * temp + 0.01 * hum * (0.99 * temp - 14.3) + 46.3;
} // Esp32Bme280::calc_thi()
