#include <Arduino.h>

const char* ntpServer = "0.dk.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

String getTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "";
    }
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}

void printLocalTime(){
  Serial.println(getTimeString());
}

void setupTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
