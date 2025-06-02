#include "mqtt/mqttManager.h"
#include <ArduinoJson.h>

MqttManager::MqttManager(){};

void MqttManager::init() {
    _client.setServer(_server, _port);

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

void MqttManager::submit(MqttTopic topic) {
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

void MqttManager::loop() {
    if (!_client.connected()) {
        init();
    }
    _client.loop();
}