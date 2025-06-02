#include <Arduino.h>
#include <WiFi.h>
#include <LEDHandler.h>

const char* ssid = "IoT_H3/4";
const char* password = "98806829";

void connectToWiFi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    pinMode(2, OUTPUT);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        blinkLED(2, 1000);
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
