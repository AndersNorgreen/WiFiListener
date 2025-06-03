#include "mqtt/mqttManager.h"
#include <ArduinoJson.h>
#include <helpers.h>

MqttManager::MqttManager(){};

void MqttManager::init() {
  _client.setServer(_server, _port);

  while (!_client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (_client.connect(_clientId)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed with state ");
      Serial.println(_client.state());
      delay(2000);
    }
  }
}

void MqttManager::initAuth() {
    _client.setServer(_server, _port);
    _espClient.setInsecure();

    while (!_client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (_client.connect(_clientId, _user, _password)) {
            Serial.println("connected!");
        } else {
            Serial.print("failed with state ");
            Serial.println(_client.state());
            delay(2000);
        }
    }
}

void MqttManager::submitTopic(MqttTopic topic) {
  JsonDocument doc;
  doc["sender"] = topic.getSender();
  doc["x"] = topic.getX();
  doc["y"] = topic.getY();

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  if (_client.connected()) {
    _client.publish(MQTT_TOPIC, jsonBuffer);
  } else {
    Serial.println("MQTT client not connected!");
  }
}

void MqttManager::submitTopics(std::vector<MqttTopic> topics) {
  JsonDocument doc;
  JsonArray data = doc["SnifferData"].to<JsonArray>();

  for (MqttTopic& topic : topics) {
    JsonObject obj = data.add<JsonObject>();
    obj["sender"] = topic.getSender();
    obj["x"] = topic.getX();
    obj["y"] = topic.getY();
  }

  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  if (len >= sizeof(jsonBuffer)) {
    Serial.println("Warning: JSON message was truncated!");
  }

  if (_client.connected()) {
    _client.publish(MQTT_TOPIC, jsonBuffer);
  } else {
    Serial.println("MQTT client not connected!");
  }
}


std::vector<MqttTopic> MqttManager::convertToTopics(std::vector<DeviceInfo> deviceData) {
  std::vector<MqttTopic> topics;
  topics.reserve(deviceData.size());

  for (const DeviceInfo& device : deviceData) {
    String sender(generateGuid());
    Coordinates coords = { device.position.x, device.position.y };
    topics.emplace_back(sender, coords);
  }

  return topics;
}

void MqttManager::loop() {
    if (!_client.connected()) {
        init();
    }
    _client.loop();
}