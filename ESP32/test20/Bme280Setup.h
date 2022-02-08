/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _BME280_SETUP_H_
#define _BME280_SETUP_H_

#include <esp32-hal-log.h>
#include "Esp32Bme280.h"
#include "Display.h"

/**
 *
 */
class Bme280Setup {
  Esp32Bme280Info_t *bme_info;

  Bme280Setup(Esp32Bme280Info_t *bme_info) {
    this->bme_info = bme_info;
  };

  void display(Display_t *disp) {
    disp->clearDisplay();
    disp->setCursor(60, 30);
    disp->setTextWrap(false);
    disp->setTextSize(2);
    disp->printf("%s: %.1f", __FILE__, this->bme_info->temp_offset);

    disp->display();
  };
};

#endif // _BME280_SETUP_H_
