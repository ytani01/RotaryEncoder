/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _BME280_H_
#define _BME280_H_

#include <esp32-hal-log.h>
#include <Adafruit_BME280.h>

typedef struct {
  // input
  uint8_t addr;
  float temp_offset;
  // output
  bool active;
  float temp;
  float hum;
  float pres;
  float thi;
} Bme280Info_t;

/**
 *
 */
class Bme280 {
 public:
  static constexpr uint8_t DEF_ADDR = 0x76;
  static constexpr float DEF_TEMP_OFFSET = -1.0;
  
  Bme280(uint8_t addr=DEF_ADDR, float temp_offset=DEF_TEMP_OFFSET);

  bool is_active();
  void set_temp_offset(float offset);
  float get_temp_offset();

  Bme280Info_t *get();

  static float calc_thi(float temp, float hum);

 protected:
  Adafruit_BME280 _bme;
  Bme280Info_t _info;
}; // class Bme280
#endif // _BME280_H_
