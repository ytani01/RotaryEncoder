# Rotary Encoder Library for ESP32
ESP32用ロータリー・エンコーダー・ライブラリ

## Description

* ESP32のパルスカウンター(PCNT)を利用
* マルチタスクで実装しているが、簡単に利用できる
* Queueを利用しているため、タイミングの制御が不要


## Sample code


## Test Program

![](docs/test-photo1.jpg)
![](docs/test-photo2.png)



## 注意事項

### タイマーは、Tickerを使う

``xTimer..()``のタイマーは、なぜかコールバック実行中に、
OLEDの``display()``が、異様に遅くなる(460ms程度 .. 通常は50ms以下)。

Tickerの場合、この問題は起きない。

### OLED(SSD1306) と BME280 の同時利用に注意！ .. マルチタスクできない!?

* もしかして、I2Cライブラリがスレッドセーフじゃない???
* OLED と BME280 は、同じタスクにする。
* この場合、BME280を``FORCED``にすると遅延が生じるので、
  使わない方がよい。
* さらにこの場合、温度が高めにでるので、-1℃ぐらい(?)の補正が必要。

## 要検討

### 戻り値が、``pdPASS``だったり、``pdTRUE``だったり..


### TWDT(Task WatchDoc Timer)

タスク毎にTWDT時間を調整しようとしたが、今回は見送り

参考コード(検討中)
```
#include <esp_task_wdt.h>
:
void task_net_mgr() {
  ESP_ERROR_CHECK(esp_task_wdt_init(60, true)); // *** 60sec
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));      // *** 必要(?)
  :
  while (true) { // main loop
    :
    ESP_ERROR_CHECK(esp_task_wdt_reset());      // *** 必要(?)
    delay(1)                // *** コンテキスト・スイッチのために必要(?)
  } // main loop
} // task_net_mgr()
```

## Reference

* Character codes
![](docs/codepage.png)

* Algorithm to convert RGB to HSV and HSV to RGB in range 0-255 for both
https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
