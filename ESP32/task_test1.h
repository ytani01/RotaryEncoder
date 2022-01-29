/**
 *
 */
#include "Esp32Task.h"

class testTask1: public Esp32Task {
 public:
  testTask1();

 protected:
  virtual void setup();
  virtual void loop();
};

testTask1::testTask1(): Esp32Task("testTask1") {
}

void testTask1::setup() {
  log_i("testTask1");
}

void testTask1::loop() {
  log_i("testTask1");
  delay(30000);
}
