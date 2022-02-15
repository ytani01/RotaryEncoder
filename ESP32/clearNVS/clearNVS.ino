#include <nvs.h>
#include <nvs_flash.h>

void clearNVS() {
int err;
err = nvs_flash_init();
Serial.print("nvs_flash_init: ");
Serial.println(err);
err = nvs_flash_erase();
Serial.print("nvs_flash_erase: ");
Serial.println(err);
}

void setup() {
Serial.begin(115200);
clearNVS();
}

void loop() {
}
