/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _ESP32_BME280_H_
#define _ESP32_BME280_H_

#include <esp32-hal-log.h>
#include <Adafruit_BME280.h>

typedef struct {
  bool active;
  uint8_t addr;
  float temp;
  float temp_offset;
  float hum;
  float pres;
  float thi;
} Esp32Bme280Info_t;

/**
 *
 */
class Esp32Bme280 {
 public:
  static constexpr uint8_t DEF_ADDR = 0x76;
  static constexpr float DEF_TEMP_OFFSET = -1.0;
  
  Esp32Bme280(uint8_t addr=DEF_ADDR, float temp_offset=DEF_TEMP_OFFSET);

  bool is_active();
  void set_temp_offset(float offset);
  float get_temp_offset();

  bool get(Esp32Bme280Info_t *info);

  static float calc_thi(float temp, float hum);

 protected:
  Adafruit_BME280 _bme;
  Esp32Bme280Info_t _info;
}; // class Esp32Bme280
#endif // _ESP32_BME280_H_
