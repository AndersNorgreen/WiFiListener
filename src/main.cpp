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

  //mqttManager.init();

  IdRoleManager& idRoleManager = IdRoleManager::getInstance();
  //idRoleManager.init();


  // Example usage of triangulationService
  // TriangulationService& triangulationService = TriangulationService::getInstance();
  // triangulationService.enableMockData(true);
  // const auto& positions = triangulationService.getDevicePositions(false);
  // Serial.println("Device positions from triangulationService:");
  // for (const auto& info : positions) {
  //   Serial.printf("MAC: %s, Position: (%d, %d)\n", info.mac, info.position.x, info.position.y);
  // }

  sleep(1);
  // ******************* Sniffing part ***********************
  SnifferService& snifferService = SnifferService::getInstance();
  snifferService.setUpEspWiFi();
  // *********************************************************
  setupEspNow();
  delay(1000); // Ensure ESP-NOW is ready
  idRoleManager.manageRoles();
  esp_wifi_set_promiscuous(false);
}

bool roleChecked = false;
String masterMac;

// Example master MAC address, replace with handshake logic to get the actual master MAC
uint8_t masterAddress[6] = {0xCC, 0xDB, 0xA7, 0x1E, 0x1F, 0x18};

int roleStatus = -1; // Declare roleStatus outside loop to persist its value
bool isNotmaster = true; // Flag to check if the device is the master

bool hasSlept = false; // Flag to check if the device has slept

void loop() {
  // ********************* Handshake part ***********************

  if (!hasSlept) {
    Serial.println("Waiting for 10 seconds before starting the loop...");
    sleep(10); // Wait for a second before starting the loop
    hasSlept = true; // Set the flag to true after the first sleep
  }

  IdRoleManager& idRoleManager = IdRoleManager::getInstance();
  roleStatus = idRoleManager.checkAndCompareRoles(masterMac);
  if (!roleChecked && roleStatus != -1) { 
        if (roleStatus == 1) {
            Serial.println("Yes, You are the Master!");
            Serial.printf("Master MAC: %s\n", masterMac.c_str());
            isNotmaster = false; // Set flag to false if this device is the master
        } else if (roleStatus == 0) {
            Serial.println("No, You are NOT the Master!");
            Serial.printf("Master MAC: %s\n", masterMac.c_str());
        }
        roleChecked = true;
    } 
  
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
  if (masterAddressSet && isNotmaster) {
    sniffAndSendService.sendSniffMessages(masterAddress);
  }

  
  delay(1000);

  // ********************* Master part ***********************
  //Serial.println("waiting for sniff data...");

  // ********************** Sniffing/Slave part ***********************
  //The slaves should have this line
  esp_wifi_set_promiscuous(true);
  //sniffAndSendService.sniff(1, 5000);
  sniffAndSendService.sniffCycleChannels(5000);
  esp_wifi_set_promiscuous(false);

  // sniffAndSendService.sendSniffMessages(masterAddress); 
}
