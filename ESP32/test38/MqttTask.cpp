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
  char buf[64];

  this->common_data = common_data;
  this->publish_interval = publish_interval;
  this->mqtt_server = mqtt_server;
  this->mqtt_port = mqtt_port;
  sprintf(buf, "%s%02X%02X%02X", topic_root.c_str(),
          _cd->netmgr_info->mac_addr[3],
          _cd->netmgr_info->mac_addr[4],
          _cd->netmgr_info->mac_addr[5]);
  this->topic_root = String(buf);
  log_i("topic_root:\"%s\"", this->topic_root.c_str());
  
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
    if ( _cd->netmgr_info->mode == NETMGR_MODE_WIFI_ON ) {
      bool ret = this->mqtt_client->connect(this->client_id.c_str(),
                                            this->user.c_str(),
                                            this->password.c_str());
      // log_i("connect(): ret=%s", ret ? "true" : "false");
    }
  } else {
    if ( cur_ms - prev_ms > this->publish_interval ) {
      prev_ms = cur_ms;

      bool ret;
      ret = this->publish("temp", _cd->bme_info->temp);
      ret = this->publish("hum", _cd->bme_info->hum);
      ret = this->publish("pres", _cd->bme_info->pres);
      ret = this->publish("thi", _cd->bme_info->thi);
      log_i("publish=>%s/ %.1f C(%.1f) %.0f %% %0.0f hPa %.1f thi",
            this->topic_root.c_str(),
            _cd->bme_info->temp, _cd->bme_info->temp_offset,
            _cd->bme_info->hum, _cd->bme_info->pres, _cd->bme_info->thi);
    }
  }
  //delay(1);
} // MqttTask::loop()
