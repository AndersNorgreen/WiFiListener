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

ServerManager serverManager;
WifiConfig wifiConfig;
MqttManager mqttManager;
TriangulationService triangulationService;
IdRoleManager idRoleManager;

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

  idRoleManager.init();
  delay(1000);
  idRoleManager.manageRoles();

  mqttManager.init();


  // Example usage of triangulationService
  triangulationService.enableMockData(true);
  const auto& positions = triangulationService.getDevicePositions(false);
  Serial.println("Device positions from triangulationService:");
  for (const auto& info : positions) {
      Serial.printf("MAC: %s, Position: (%d, %d)\n", info.mac, info.position.x, info.position.y);
  }

}

bool roleChecked = false;

void loop() {
  int roleStatus = idRoleManager.checkAndCompareRoles();
  if (!roleChecked && roleStatus != -1) { 
        if (roleStatus == 1) {
            Serial.println("Yes, You are the Master!");
        } else if (roleStatus == 0) {
            Serial.println("No, You are NOT the Master!");
        }
        roleChecked = true;
    }
  delay(1000);
}
