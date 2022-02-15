#include <esp32-hal-log.h>
#include "FS.h"
#include "SPIFFS.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  File root = fs.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }

  log_i("%s/", root.name());
  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      log_i("%s/", file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      time_t ts = file.getLastWrite();
      struct tm *ti = localtime(&ts);
      log_i("%5'd %4d/%02d/%02d %02d:%02d:%02d %s",
            file.size(),
            ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday,
            ti->tm_hour, ti->tm_min, ti->tm_sec,
            file.name());
    }
    file = root.openNextFile();
  }
}

void setup(){
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
    log_e("SPIFFS mount failed");
    return;
  }

  SPIFFS.remove("/temp");
  // SPIFFS.remove("/temp1");
  // SPIFFS.remove("/test");
  // SPIFFS.remove("/wifi.txt");

  listDir(SPIFFS, "/", 1);
}

void loop() {
  delay(1000);
}
