/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _OLED_TASK_H_
#define _OLED_TASK_H_

#include <esp32-hal-log.h> // XXX
#include "common.h"
#include "Oled.h"
#include "Esp32Task.h"
#include "Esp32RotaryEncoder.h"
#include "Esp32Button.h"
#include "Esp32NetMgrTask.h"
#include "Esp32NtpTask.h"
#include "Esp32Bme280.h"
#include "OledMenu.h"

/**
 * 表示データ
 */
typedef struct {
  char cmd[64];
  Mode_t mode;
  Esp32NetMgrInfo_t *ni;
  Esp32NtpTaskInfo_t *ntp_info;
  Esp32RotaryEncoderInfo_t *ri1;
  Esp32ButtonInfo_t *bi1;
  OledMenu *menu;
} DispData_t;

/**
 *
 */
class OledTask: public Esp32Task {
 public:
  Display_t *disp;
  Esp32NetMgrTask **pNetMgrTask;

  OledTask(DispData_t *disp_data);

  virtual void setup();
  virtual void loop();

protected:
  DispData_t *disp_data;
  Esp32Bme280 *_bme;

  void drawTemp(int x, int y, float temp);
  void drawHum(int x, int y, float hum);
  void drawPres(int x, int y, float press);
  void drawThi(int x, int y, float thi);
  void drawWiFi(int x, int y, Esp32NetMgrInfo_t *ni);
  void drawDateTime(int x, int y, struct tm *ti);
}; // class OledTask
#endif // _OLED_TASK_H_
