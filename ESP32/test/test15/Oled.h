/**
 *
 */
#ifndef _OLED_H_
#define _OLED_H_

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

typedef Adafruit_SSD1306 Display_t;

static constexpr uint16_t OLED_DISP_W = 128;
static constexpr uint16_t OLED_DISP_H = 64;
static constexpr uint16_t OLED_CH_W = 6;
static constexpr uint16_t OLED_CH_H = 8;

#endif // _OLED_H_
