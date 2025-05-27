#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "mqttTopic.h"
#include "config.h"

class MqttManager {
    public: 
        MqttManager();

        void init();
        void submit(MqttTopic topic);
        void loop();
    private:
        const char* _server = MQTT_SERVER;
        uint16_t _port = MQTT_PORT;
        const char* _clientId = MQTT_CLIENT_ID;
        const char* _user = MQTT_USER;
        const char* _password = MQTT_PASSWORD;

        WiFiClient _espClient;
        PubSubClient _client = PubSubClient(_espClient);
};

#endif