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
           String mqtt_server, int mqtt_port=1883,
           String topic_root="esp32",
           String client_id="esp32client", String user="", String passwd="")
    : Task("MQTT Task") {
    this->common_data = common_data;
    this->mqtt_server = mqtt_server;
    this->mqtt_port = mqtt_port;
    this->topic_root = topic_root;
    this->client_id = client_id;
    this->user = user;
    this->password = password;
  };

 protected:
  virtual void setup() {
    this->wifi_client = new WiFiClient();
    this->mqtt_client = new PubSubClient(*(this->wifi_client));

    this->mqtt_client->setServer(this->mqtt_server.c_str(), this->mqtt_port);
  };

  virtual void loop() {
    static unsigned long prev_ms = millis();
    unsigned long cur_ms = millis();

    this->mqtt_client->loop();

    if ( !this->mqtt_client->connected() ) {
      if ( this->common_data->netmgr_info->mode == NETMGR_MODE_WIFI_ON ) {
        bool ret = this->mqtt_client->connect(this->client_id.c_str(),
                                              this->user.c_str(),
                                              this->password.c_str());
        log_i("connect(): ret=%s", ret ? "true" : "false");
      }
    } else {
      if ( cur_ms - prev_ms > 30000 ) {
        prev_ms = cur_ms;
        String topic;
        char buf[8];
        bool ret;

        topic = this->topic_root + "/temp";
        sprintf(buf, "%.1f", common_data->bme_info->temp);
        ret = this->mqtt_client->publish(topic.c_str(), buf);
        log_i("publish(%s:%s): ret=%s",
              topic.c_str(), buf, ret ? "true" : "false");

        topic = this->topic_root + "/hum";
        sprintf(buf, "%.1f", common_data->bme_info->hum);
        ret = this->mqtt_client->publish(topic.c_str(), buf);
        log_i("publish(%s:%s): ret=%s",
              topic.c_str(), buf, ret ? "true" : "false");

        topic = this->topic_root + "/pres";
        sprintf(buf, "%.1f", common_data->bme_info->pres);
        ret = this->mqtt_client->publish(topic.c_str(), buf);
        log_i("publish(%s:%s): ret=%s",
              topic.c_str(), buf, ret ? "true" : "false");
      }
    }
    //delay(1);
  };
}; // class MqttTask

#endif // _MQTT_TASK_
