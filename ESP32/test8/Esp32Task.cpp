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

  strcpy(this->conf.name, name2.c_str());
  log_i("conf.name=%s", conf.name);
  this->conf.handle = NULL;
  this->conf.stack_size = stack_size;
  this->conf.priority = priority;
  this->conf.wdt_sec = wdt_sec;
  this->conf.core = core;

  this->_active = false;
  
  delay(100); // XXX これがないと、危ない(!?)
} // Esp32Task::Esp32Task()

/**
 *
 */
void Esp32Task::start() {
  BaseType_t ret = xTaskCreateUniversal(Esp32Task::call_task_main,
                                        this->conf.name,
                                        this->conf.stack_size,
                                        this,
                                        this->conf.priority,
                                        &(this->conf.handle),
                                        this->conf.core);
  log_i("Start: %s: ret=%d", this->conf.name, ret);
  if ( ret != pdPASS ) {
    log_e("ret=%d .. HALT", ret);
    while (true) { // !!! halt !!!
      delay(1);
    }
  }
  delay(100);
  return;
} // Esp32Task::start()

/**
 *
 */
bool Esp32Task::is_active() {
  return this->_active;
}

/**
 *
 */
void Esp32Task::__task_main() {
  // Watchdog Timer の初期化
  ESP_ERROR_CHECK(esp_task_wdt_init(this->conf.wdt_sec, true));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  this->_active = false;

  this->setup();
  delay(1);

  this->_active = true;

  while (true) { // main loop
    ESP_ERROR_CHECK(esp_task_wdt_reset());

    this->loop();

    delay(1);
  } // main loop

  this->_active = false;
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
