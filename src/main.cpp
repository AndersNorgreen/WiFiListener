#include <Arduino.h>
#include <helpers.h>
#include <server/serverManager.h>
#include <mqtt/mqttManager.h>
#include <idGenerator.h>
#include "config.h"
#include "server/wifiConfig.h"
#include "triangulationService.h"

ServerManager serverManager;
WifiConfig wifiConfig;
MqttManager mqttManager;
TriangulationService triangulationService;
IdGenerator idGenerator;

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

  idGenerator.initId();
  int ID = idGenerator.getRandomID();

  mqttManager.init();


  // Example usage of triangulationService
  triangulationService.enableMockData(true);
  const auto& positions = triangulationService.getDevicePositions(false);
  Serial.println("Device positions from triangulationService:");
  for (const auto& info : positions) {
      Serial.printf("MAC: %s, Position: (%d, %d)\n", info.mac, info.position.x, info.position.y);
  }

}

void loop() {
}
