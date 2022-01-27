# Rotary Encoder Library for ESP32
ESP32用ロータリー・エンコーダー・ライブラリ

## Description

ESP32のパルスカウンター(PCNT)を使用した、
ロータリー・エンコーダー・ライブラリ

## Class

* Esp32PcntRotaryEncoder


## Hardware

### OLED: SSD1306

```
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
:
Adafruit_SSD1306 *disp;
:
setup() {
  disp = new Adafruit_SSD1306(128, 64);
  disp->display(); // Initially, display Adafruit logo
}

void task(..) {
  disp->clearDisplay();
  disp->drawRect(..);
  :
  disp->display();
  
}
```

## 注意

### タイマーは、Tickerを使う

xTimer..のタイマーは、なぜかコールバック実行中に、
OLEDの display() が、以上に遅くなる(460ms程度)。

Tickerの場合、問題は起きない。


## Reference

* Algorithm to convert RGB to HSV and HSV to RGB in range 0-255 for both
https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
