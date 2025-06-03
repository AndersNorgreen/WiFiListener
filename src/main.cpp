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
TriangulationService triangulationService;
IdRoleManager idRoleManager;
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

  // TODO get mqttServer running
  //mqttManager.init();

  String myMac = WiFi.macAddress();
  idRoleManager.updateDeviceList(const_cast<char*>(myMac.c_str()), 0);

  SnifferService& snifferService = SnifferService::getInstance();
  snifferService.setUpEspWiFi();
  setupEspNow();
  esp_wifi_set_promiscuous(false); 
}

void loop() {
  esp_wifi_set_promiscuous(true);
  //sniffAndSendService.sniff(1, 5000);
  sniffAndSendService.sniffCycleChannels(5000);
  esp_wifi_set_promiscuous(false);

  //TODO get the master address from the coordinated service
  uint8_t masterAddress[6] = {0xCC, 0xDB, 0xA7, 0x1C, 0xA8, 0x6C}; 
  sniffAndSendService.sendSniffMessages(masterAddress);

  // Call the triangulation service to process measurements
  idRoleManager.updateDeviceList("AA:BB:CC:DD:EE:FF", 1); // TODO: replace with actual values, mac: is mac address from the sniffer, id: is the device ID of the tracker sending the message
  triangulationService.addMeasurement("AA:BB:CC:DD:EE:FF", "11:22:33:44:55:66", -70); // TODO: replace with actual values

  // TODO: add a timer so that this is not called every loop iteration
  // If this device is the master, get device positions and send them to the MQTT broker
  if (idRoleManager.isMaster()) { 
    // get data from triangulationService and send it to the MQTT broker
    const auto& devicePositions = triangulationService.getDevicePositions(true);

    // TODO: send device positions to MQTT broker
  }
}
