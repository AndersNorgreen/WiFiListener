#include <Arduino.h>
#include <esp_now.h>
#include <helpers.h>
#include <server/serverManager.h>
#include <mqtt/mqttManager.h>
#include <id/idGenerator.h>
#include <id/idRoleManager.h>
#include "config.h"
#include "server/wifiConfig.h"
#include "triangulationService.h"
#include <WiFi.h>
#include <sniffAndSendService.h>
#include <espNowHandler.h>
#include <sniffer.h>
#include <esp_wifi.h>

ServerManager serverManager;
WifiConfig wifiConfig;
MqttManager mqttManager;
SniffAndSendService sniffAndSendService;

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random()); // Helps generate GUIDs

  initLittleFS();
  
  wifiConfig.setSSID(CURRENT_SSID);
  wifiConfig.setPassword(CURRENT_PASSWORD);
  wifiConfig.setIpAddress(CURRENT_API_IP);
  wifiConfig.setGateway(CURRENT_GATEWAY);
  wifiConfig.setSubnet("255.255.255.0");
  
  serverManager.updateWifiConfig(wifiConfig);
  serverManager.initServer();


  IdRoleManager& idRoleManager = IdRoleManager::getInstance();
  idRoleManager.init();
  delay(1000);
  idRoleManager.manageRoles();

  mqttManager.init();


  // Example usage of triangulationService
  TriangulationService& triangulationService = TriangulationService::getInstance();
  triangulationService.enableMockData(true);
  const auto& positions = triangulationService.getDevicePositions(false);
  Serial.println("Device positions from triangulationService:");
  for (const auto& info : positions) {
      Serial.printf("MAC: %s, Position: (%d, %d)\n", info.mac, info.position.x, info.position.y);
  }

  SnifferService& snifferService = SnifferService::getInstance();
  snifferService.setUpEspWiFi();
  setupEspNow();
  esp_wifi_set_promiscuous(false);
}

bool roleChecked = false;
String masterMac;

void loop() {
  IdRoleManager& idRoleManager = IdRoleManager::getInstance();
  int roleStatus = idRoleManager.checkAndCompareRoles(masterMac);
  if (!roleChecked && roleStatus != -1) { 
        if (roleStatus == 1) {
            Serial.println("Yes, You are the Master!");
            Serial.printf("Master MAC: %s\n", masterMac.c_str());
        } else if (roleStatus == 0) {
            Serial.println("No, You are NOT the Master!");
            Serial.printf("Master MAC: %s\n", masterMac.c_str());
        }
        roleChecked = true;
    }
  delay(1000);

  esp_wifi_set_promiscuous(true);
  //sniffAndSendService.sniff(1, 5000);
  sniffAndSendService.sniffCycleChannels(5000);
  esp_wifi_set_promiscuous(false);

  uint8_t masterAddress[6] = {0};
  // Convert masterMac string (format: "AA:BB:CC:DD:EE:FF") to uint8_t array
  static bool masterAddressSet = false;
  
  if (!masterAddressSet) {
    if (sscanf(masterMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &masterAddress[0], &masterAddress[1], &masterAddress[2],
               &masterAddress[3], &masterAddress[4], &masterAddress[5]) == 6) {
        masterAddressSet = true;
    } else {
        Serial.println("Invalid master MAC address format!");
        Serial.printf("Master MAC: %s\n", masterMac.c_str());
    }
  }

  uint8_t selfMac[6];
  esp_read_mac(selfMac, ESP_MAC_WIFI_STA);
  bool isOwnMac = memcmp(masterAddress, selfMac, 6) == 0;

  if (masterAddressSet && !isOwnMac) {
    sniffAndSendService.sendSniffMessages(masterAddress);
  }
}
