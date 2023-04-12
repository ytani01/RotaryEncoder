/**
 * Copyright (c) 2023 Yoichi Tanibayahsi
 */
#ifndef _TASK_FOO_H_
#define _TASK_FOO_H_

#include "Task.h"

/**
 *
 */
class Task_Foo: public Task {

 public:
  Task_Foo(String name);

 protected:
  virtual void setup();
  virtual void loop();
  
}; // class Task_Foo
#endif // _TASK_FOO_H_
