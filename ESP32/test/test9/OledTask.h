/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _OLED_TASK_H_
#define _OLED_TASK_H_

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "Esp32Task.h"
#include "Esp32RotaryEncoderTask.h"
#include "Esp32NetMgrTask.h"

/**
 *
 */
class OledTask: public Esp32Task {
 public:
  static const uint16_t DISP_W = 128;
  static const uint16_t DISP_H = 64;
  static const uint16_t CH_W = 6;
  static const uint16_t CH_H = 8;
  static const uint16_t FRAME_W = 3;

  static const uint32_t CMD_BUF_SIZE = 128;
  static const uint32_t CMD_NAME_SIZE = 8;

  Adafruit_SSD1306 *disp;

  Esp32RotaryEncoderTask **pReTask;
  Esp32NetMgrTask **pNetMgrTask;

  OledTask(Esp32RotaryEncoderTask **pReTask, Esp32NetMgrTask **pNetMgrTask,
           QueueHandle_t in_que);

  virtual void setup();
  virtual void loop();

 private:
  char _buf[CMD_BUF_SIZE];
  char _cmd[CMD_NAME_SIZE];
}; // class OledTask
#endif // _OLED_TASK_H_
