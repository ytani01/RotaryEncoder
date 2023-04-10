/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 *
 * Library: "PubSubClien" by Nick O'Leary
 */
#ifndef _MQTT_TASK_
#define _MQTT_TASK_

#include <WiFi.h>
#include <PubSubClient.h>
#include "common.h"

class MqttTask: public Task {
 public:
  CommonData_t *common_data;
  String mqtt_server;
  int mqtt_port;
  String topic_root;
  String client_id;
  String user;
  String password;

  WiFiClient *wifi_client;
  PubSubClient *mqtt_client;
  
  MqttTask(CommonData_t *common_data,
           String mqtt_server="mqtt", int mqtt_port=1883,
           String topic_root="esp32",
           String client_id="esp32client", String user="", String passwd="");

 protected:
  virtual void setup();
  virtual void loop();
  virtual bool publish(String topic_sub, float value);
}; // class MqttTask

#endif // _MQTT_TASK_
