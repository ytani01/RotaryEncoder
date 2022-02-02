/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledTask.h"
#include "Esp32NtpTask.h"
#include <Adafruit_BME280.h>

/**
 * OLED(SSD1306)と、BME280は、同一タスクにしないとハマる!!??
 */
Adafruit_BME280 bme;
bool bme_active = false;

/**
 *
 */
OledTask::OledTask(DispData_t *disp_data):
  Esp32Task("OLED(SSD1306)", 4*1024, 1) {

  this->disp_data = disp_data;
  this->disp = NULL;
}

/**
 *
 */
void OledTask::setup() {
  if ( bme.begin(0x76) ) {
    bme_active = true;
  }
  log_i("bme_active=%d", bme_active);
  // bme.setSampling(Adafruit_BME280::MODE_FORCED);  // FORCEDにしない方がよい!?

  this->disp = new Adafruit_SSD1306(DISP_W, DISP_H);
  this->disp->begin(SSD1306_SWITCHCAPVCC, 0x3C);

  this->disp->display(); // display Adafruit Logo
  delay(1000);

  this->disp->clearDisplay();
  //this->disp->cp437(true);
  this->disp->setTextColor(WHITE);
  this->disp->setTextWrap(false);

}

/**
 *
 */
void OledTask::loop() {
  static unsigned long prev_ms = millis();
  static int d_ms_max = 0;
  static unsigned long d_ms_max_ms = millis();

  unsigned long cur_ms = millis();
  int d_ms = cur_ms - prev_ms;
  prev_ms = cur_ms;

  // cmd
  String cmd = String(disp_data->cmd);
  if ( cmd != "" ) {
    log_i("cmd=%s", disp_data->cmd);
  }
  static bool clr_flag = false;
  if ( cmd == "clear" ) {
    clr_flag = true;
  }

  // msec per display
  if ( d_ms >= d_ms_max || cur_ms - d_ms_max_ms >= 3000 ) {
    if ( d_ms != d_ms_max ) {
      log_i("d_ms=%d %d:d_ms_max=%d",
            d_ms, cur_ms - d_ms_max_ms, d_ms_max);
      d_ms_max = d_ms;
    }
    d_ms_max_ms = cur_ms;
  }

  // get temp, hum, pres
  static float temp, hum, pres;
  static unsigned long prev_temp_ms = millis() - 5000;
  if ( bme_active ) {
    if ( cur_ms - prev_temp_ms > 5000 ) {
      prev_temp_ms = cur_ms;
    
      // bme.takeForcedMeasurement(); // XXX
      temp = bme.readTemperature() - 1.1;
      hum = bme.readHumidity();
      pres = bme.readPressure() / 100.0;
      log_i("%.1f^C %.0f%% %0.0fhPa", temp, hum, pres);
    }
  } // if (bme_active)

  Esp32NetMgrInfo_t *ni = this->disp_data->ni;
  Esp32ButtonInfo_t *bi1 = this->disp_data->bi1;
  Esp32RotaryEncoderInfo_t *ri1 = this->disp_data->ri1;

  /**
   * draw, fill ..
   */
  // clear
  D->clearDisplay();
  D->fillRect(0,0, DISP_W, DISP_H, WHITE);
  D->fillRect(FRAME_W, FRAME_W,
              DISP_W - FRAME_W * 2, DISP_H - FRAME_W * 2,
              BLACK);

  if ( clr_flag ) {
    D->display();
    return;
  }
  clr_flag = false;
  
  int x, y, r;
  // cmd
  if ( cmd != "" ) {
    x = 82;
    y = 2;
    D->setCursor(x, y);
    D->setTextSize(1);
    D->printf("%s", cmd.c_str());
  }
  
  // msec per display
  char buf[16];
  sprintf(buf, "%dms", d_ms_max);
  x = DISP_W - CH_W * 4 - 2;
  y = DISP_H - CH_H - 2;
  D->setTextSize(1);
  D->setCursor(x, y);
  D->write(buf);

  // temp, hum, press
  x = 9;
  y = CH_H + 5;
  D->setCursor(x, y);
  D->setTextSize(3);
  D->printf("%2.0f", temp);
  D->setTextSize(1);
  D->printf("%cC ", (char)247);
  D->setCursor(x + CH_W * 2 * 3 - 5, y + CH_H);
  D->setTextSize(2);
  D->printf(".%d", int((temp - int(temp))*10));
      
  x += CH_W * 3 * 3 + 8;
  D->setCursor(x, y);
  D->setTextSize(3);
  D->printf("%2.0f", hum);
  D->setCursor(x + CH_W * 3 * 2, y + CH_H + 1);
  D->setTextSize(2);
  D->printf("%%");

  // Circle
  if ( ri1->angle_max != 0 ) {
    x = DISP_W * ri1->angle / ri1->angle_max;
    y = 35 - bi1->push_count * 4;
    r = 3 + bi1->repeat_count * 2;
    D->fillCircle(x, y, r, WHITE);
  }

  // WiFi SSID
  x = 2;
  y = 2;
  D->setTextSize(1);
  D->setCursor(x, y);
  if ( ni->mode == NETMGR_MODE_WIFI_ON ) {
    D->printf("SSID:%s", ni->ssid.c_str());
  } else if ( ni->mode == NETMGR_MODE_AP_LOOP ) {
    D->printf("AP:%s", ni->ssid.c_str());
  } else {
    D->printf("WiFi..");
  }

  // Date, Time
  time_t t = time(NULL);
  struct tm *ti = localtime(&t);
  if ( ti->tm_year + 1900 > 2000 ) {
    char date_str[32], time_str[16];
    strftime(date_str, sizeof(date_str), "%Y/%m/%d(%a)", ti);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", ti);

    x = 4;
    y = DISP_H - CH_H * 2 - 1;
    D->setTextSize(2);
    D->setCursor(x, y);
    D->write(time_str);

    y = y - CH_H - 1;
    D->setTextSize(1);
    D->setCursor(x, y);
    D->write(date_str);
  } // if (ti)

  D->display();
  if ( d_ms > 50 ) {
    log_w("display(): d_ms=%d", d_ms);
  }
} // OledTask::loop()
