/**
 * Copyright (c) 2023 Yoichi Tanibayashi
 */
#include "Mode.h"

class Mode_Main: public Mode {

public:
  Mode_Main(String name, Display_t* disp);

  virtual void setup();

  virtual void enter();

  virtual void loop(unsigned long cur_ms);

  virtual void exit();

  //virtual void display(Display_t *disp);
  
}; // class Mode_Main
