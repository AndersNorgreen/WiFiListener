#include <Arduino.h>
#include <helpers.h>
#include <serverManager.h>

#include "config.h"
#include "wifiConfig.h"

ServerManager serverManager;
WifiConfig wifiConfig;

void setup() {
  Serial.begin(115200);

  initLittleFS();
  
  wifiConfig.setSSID(CURRENT_SSID);
  wifiConfig.setPassword(CURRENT_PASSWORD);
  wifiConfig.setIpAddress(CURRENT_API_IP);
  wifiConfig.setGateway(CURRENT_GATEWAY);
  wifiConfig.setSubnet("255.255.255.0");
  
  serverManager.updateWifiConfig(wifiConfig);
  serverManager.initServer();
}

void loop() {
}