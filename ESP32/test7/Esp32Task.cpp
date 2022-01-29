/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "Esp32Task.h"

/**
 *
 */
Esp32Task::Esp32Task(String name,
                     uint32_t stack_size, UBaseType_t priority, uint32_t wdt_sec,
                     UBaseType_t core) {
  String name2 = name;
  log_i("name2=%s", name2.c_str());
  if ( name2.length() > ESP32_TASK_NAME_SIZE ) {
    name2 = name2.substring(0, ESP32_TASK_NAME_SIZE);
  }
  log_d("name2=%s", name2.c_str());

  strcpy(this->info.name, name2.c_str());
  log_i("info.name=%s", info.name);
  this->info.handle = NULL;
  this->info.stack_size = stack_size;
  this->info.priority = priority;
  this->info.wdt_sec = wdt_sec;
  this->info.core = core;

  delay(100); // XXX これがないと、危ない(!?)
} // Esp32Task::Esp32Task()

/**
 *
 */
void Esp32Task::start() {
  BaseType_t ret = xTaskCreateUniversal(Esp32Task::call_task_main,
                                        this->info.name,
                                        this->info.stack_size,
                                        this,
                                        this->info.priority,
                                        &(this->info.handle),
                                        this->info.core);
  log_i("Start: %s: ret=%d", this->info.name, ret);
  if ( ret != pdPASS ) {
    log_e("ret=%d .. HALT", ret);
    while (true) { // !!! halt !!!
      delay(1);
    }
  }
  delay(100);
  return;
}

/**
 *
 */
void Esp32Task::__task_main() {
  // Watchdog Timer の初期化
  ESP_ERROR_CHECK(esp_task_wdt_init(this->info.wdt_sec, true));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  this->setup();

  delay(1);

  while (true) { // main loop
    ESP_ERROR_CHECK(esp_task_wdt_reset());

    this->loop();

    delay(1);
  } // main loop
  vTaskDelete(NULL);
} // Esp32Task::task_main()

/**
 *
 */
void Esp32Task::setup() {
  log_d("");
} // Esp32Task::setup()

/**
 *
 */
void Esp32Task::loop() {
  log_d("");
  delay(1000);
} // Esp32Task::loop()

/**
 * TaskFunctionは、staticでなければならいので、
 * static関数を定義して、その引数に``this``を渡して、
 * task_main()を呼び出す
 */
void Esp32Task::call_task_main(void *this_instance) {
  static_cast<Esp32Task *>(this_instance)->__task_main();
} // Esp32Task::call_task_main()
