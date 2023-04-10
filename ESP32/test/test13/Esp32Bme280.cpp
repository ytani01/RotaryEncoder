/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32Bme280.h"

#define _Info this->_info
#define _Bme this->_bme

/**
 *
 */
Esp32Bme280::Esp32Bme280(uint8_t addr, float temp_offset) {
  _Info.addr = addr;
  _Info.temp_offset = temp_offset;

  _Info.active = _Bme.begin(_Info.addr);

  log_i("active=%d", _Info.active);
} // Esp32Bme280::Esp32Bme280()

/**
 *
 */
bool Esp32Bme280::is_active() {
  return _Info.active;
} // Esp32Bme280::is_active();

/**
 *
 */
void Esp32Bme280::set_temp_offset(float temp_offset) {
  _Info.temp_offset = temp_offset;
} // Esp32Bme280::set_temp_offset()

/**
 *
 */
float Esp32Bme280::get_temp_offset() {
  return _Info.temp_offset;
} // Esp32Bme280::get_temp_offset()

/**
 * @param [out] info
 */
bool Esp32Bme280::get(Esp32Bme280Info_t *info) {
  if ( ! _Info.active ) {
    log_w("active=%d", _Info.active);
    return false;
  }

  _Info.temp = _Bme.readTemperature() + _Info.temp_offset;
  _Info.hum = _Bme.readHumidity();
  _Info.pres = _Bme.readPressure() / 100;

  _Info.thi = calc_thi(_Info.temp, _Info.hum);

  *info = _Info;

  log_i("%.1f C %.0f%% %0.0f hPa %.1f thi",
        _Info.temp, _Info.hum, _Info.pres, _Info.thi);
  return true;
} // Esp32Bme280::get()

/** static function
 *
 */
float Esp32Bme280::calc_thi(float temp, float hum) {
  return  0.81 * temp + 0.01 * hum * (0.99 * temp - 14.3) + 46.3;
} // Esp32Bme280::calc_thi()
