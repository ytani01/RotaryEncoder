/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "commonlib.h"

/**
 *
 */
char* get_mac_addr(char* mac_str) {
  uint8_t mac_addr[6];

  esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
  sprintf(mac_str, "%02x%02x%02x%02x%02x%02x",
          mac_addr[0], mac_addr[1], mac_addr[2],
          mac_addr[3], mac_addr[4], mac_addr[5]);
  log_d("MacAddr=%s", mac_str);

  return mac_str;
} // get_mac_addr()

/**
 *
 */
String get_mac_addr_String() {
  char mac_addr[13];
  return String(get_mac_addr(mac_addr));
}


/**
 *
 */
void task_delay(uint32_t ms) {
  //vTaskDelay(ms / portTICK_PERIOD_MS);

  //XXX TBD: vTaskDelay()は不正確？？
  portTickType lastWakeTime = xTaskGetTickCount();
  vTaskDelayUntil( &lastWakeTime, ms / portTICK_PERIOD_MS );
} // task_delay()
