/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "MqttTask.h"

/** constructor
 *
 */
MqttTask::MqttTask(CommonData_t *common_data,
                   unsigned long publish_interval,
                   String mqtt_server, int mqtt_port,
                   String topic_root,
                   String client_id, String user, String password)
  : Task("MQTT Task") {
  this->common_data = common_data;
  this->publish_interval = publish_interval;
  this->mqtt_server = mqtt_server;
  this->mqtt_port = mqtt_port;
  this->topic_root = topic_root;
  this->client_id = client_id;
  this->user = user;
  this->password = password;
} // MqttTask::MqttTask()

/** protected virtual
 *
 */
void MqttTask::setup() {
  this->wifi_client = new WiFiClient();
  this->mqtt_client = new PubSubClient(*(this->wifi_client));
  
  this->mqtt_client->setServer(this->mqtt_server.c_str(), this->mqtt_port);
} // MqttTask::setup()

/** protected virtual
 *
 */
bool MqttTask::publish(String topic_sub, float value) {
  String topic = this->topic_root + "/" + topic_sub;
  char buf[8];
  sprintf(buf, "%.2f", value);
  bool ret = this->mqtt_client->publish(topic.c_str(), buf);
  log_i("publish(%s:%s): ret=%s", topic.c_str(), buf, ret ? "true" : "false");
  return ret;
} // MqttTask::publish()

/** protected virtual
 *
 */
void MqttTask::loop() {
  static unsigned long prev_ms = 0;
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
    if ( cur_ms - prev_ms > this->publish_interval ) {
      prev_ms = cur_ms;

      bool ret;
      ret = this->publish("temp", common_data->bme_info->temp);
      ret = this->publish("hum", common_data->bme_info->hum);
      ret = this->publish("pres", common_data->bme_info->pres);
      ret = this->publish("thi", common_data->bme_info->thi);
    }
  }
  //delay(1);
} // MqttTask::loop()
