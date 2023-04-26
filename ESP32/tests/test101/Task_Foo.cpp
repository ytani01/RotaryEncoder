/**
 * Copyright (c) 2023 Yoichi Tanibayahsi
 */
#include "Task_Foo.h"

/**
 *
 */
Task_Foo::Task_Foo(String name)
  :Task(name) {
} // TaskFoo::TaskFoo()

/**
 *
 */
void Task_Foo::setup() {
  log_i("");
} // Task_Foo::setup()

/**
 *
 */
void Task_Foo::loop() {
  static unsigned long prev_ms = millis();
  unsigned long cur_ms = millis();
  unsigned long d_ms = cur_ms - prev_ms;

  log_d("'%s':%u:%u", this->conf.name, cur_ms, d_ms);

  prev_ms = cur_ms;
  task_delay(60 * 1000);
} // Task_Foo::loop()
