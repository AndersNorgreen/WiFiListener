#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "mqttTopic.h"
#include "config.h"
#include <vector>
#include <WiFiClientSecure.h>
#include <triangulationService.h>

class MqttManager {
    public: 
        MqttManager();

        void init();
        void initAuth();
        void submitTopic(MqttTopic topic);
        void submitTopics(std::vector<MqttTopic> topics);
        std::vector<MqttTopic> convertToTopics(std::vector<DeviceInfo> deviceData);
        void loop();
    private:
        const char* _server = MQTT_SERVER;
        uint16_t _port = MQTT_PORT;
        const char* _clientId = MQTT_CLIENT_ID;
        const char* _user = MQTT_USER;
        const char* _password = MQTT_PASSWORD;

        WiFiClientSecure _espClient;
        PubSubClient _client = PubSubClient(_espClient);
};

#endif