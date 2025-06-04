#include <Arduino.h>
#include <esp_now.h>
#include <struct_message.h>
#include <WiFi.h>
#include <timeHandler.h>
#include <triangulationService.h>
#include "id/idRoleManager.h"

struct_message myData;
struct_message incomingReadings;

struct_sniff_message sniffData;
struct_sniff_message incomingSniffData;

String incomingTime;
uint8_t incomingMacAddress[6];

esp_now_peer_info_t peerInfo;

// create an array of broadcast addresses, TODO get this from the coordinator service
uint8_t broadcastAddresses[4][6] = { 
  {0xCC, 0xDB, 0xA7, 0x12, 0x51, 0x0C}, 
  {0xCC, 0xDB, 0xA7, 0x1E, 0x1F, 0x18}, 
  {0xCC, 0xDB, 0xA7, 0x1D, 0xFD, 0xFC}, 
  {0xCC, 0xDB, 0xA7, 0x1C, 0xA8, 0x6C} 
};

int myMacIndex = -1;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Received data: ");
  for (int i = 0; i < len; i++) {
    Serial.printf("%02X", incomingData[i]);
    if (i < len - 1) Serial.print(" ");
  }
  Serial.println();

  if (len == sizeof(struct_message)) {
        IdRoleManager &idRoleManager = IdRoleManager::getInstance();
    if (idRoleManager.getReadyToCompare()) {
      Serial.println("Ready to compare roles, managing roles now...");
    } else {
      Serial.println("Not ready to compare roles, waiting...");
      idRoleManager.manageRoles();
    }

    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.println("Received struct_message:");
    incomingTime = String(incomingReadings.id);
    memcpy(incomingMacAddress, incomingReadings.macAddress, 6);

    Serial.print("Packet received from: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", incomingMacAddress[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    Serial.print("Time: ");
    Serial.println(incomingTime);
    Serial.print("Size of incomingReadings: ");
    Serial.println(sizeof(incomingReadings));
    Serial.println("Data received successfully.");

    // handshake part
    struct struct_message incomingMsg;
    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.printf("ID received from: %s - ID: %s\n", macStr, incomingMsg.id);

    idRoleManager.updateDeviceInfoTracker(macStr, incomingMsg.id);
    idRoleManager.setReadyToCompare(true);

  } else if (len == sizeof(struct_sniff_message)) {
    memcpy(&incomingSniffData, incomingData, sizeof(incomingSniffData));
    Serial.println("Received struct_sniff_message:");
    // Print fields from incomingSniffData as needed
    Serial.print("MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", incomingSniffData.macAddress[i]);
      if (i < 5) Serial.print(":");
    }
    String masterMac;
    TriangulationService& triangulationService = TriangulationService::getInstance();

    char deviceMacStr[18];
    char trackerMacStr[18];
    snprintf(deviceMacStr, sizeof(deviceMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
          incomingSniffData.macAddress[0], incomingSniffData.macAddress[1], incomingSniffData.macAddress[2],
          incomingSniffData.macAddress[3], incomingSniffData.macAddress[4], incomingSniffData.macAddress[5]);
    snprintf(trackerMacStr, sizeof(trackerMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
          incomingSniffData.device_macAddress[0], incomingSniffData.device_macAddress[1], incomingSniffData.device_macAddress[2],
          incomingSniffData.device_macAddress[3], incomingSniffData.device_macAddress[4], incomingSniffData.device_macAddress[5]);
    
    Serial.printf("Device MAC: %s, Tracker MAC: %s, RSSI: %d\n", deviceMacStr, trackerMacStr, atoi(incomingSniffData.RSSI));
    
    triangulationService.addMeasurement(
      deviceMacStr,
      trackerMacStr,
      atoi(incomingSniffData.RSSI));

  } else {
    Serial.print("Unknown data size received: ");
    Serial.println(len);
  }
}

static int getMyIndexInList() {
    uint8_t myMac[6];
    WiFi.macAddress(myMac);
    for (int i = 0; i < 4; i++) {
        bool match = true;
        for (int j = 0; j < 6; j++) {
            if (broadcastAddresses[i][j] != myMac[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1; // Not found
}

void buildMessage() {
  // Set values to send
  strcpy(myData.id, getTimeString().c_str());
  // Send the esp MAC address
  WiFi.macAddress(myData.macAddress);
  // Alternatively, you can use the following line to send the MAC address of the Wifi router:
  // memcpy(myData.macAddress, WiFi.BSSID(), 6);
}

void sendSniffMessage(const struct_sniff_message &sniffMsg, uint8_t masterAddress[6] ) {
    if (!masterAddress) {
      Serial.println("masterAddress is null!");
      return;
  }
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &sniffMsg, sizeof(sniffMsg));
  Serial.print("Sending sniff data to: ");
  for (int j = 0; j < 6; j++) {
    Serial.printf("%02X", masterAddress[j]);
    if (j < 5) Serial.print(":");
  }
  if (result == ESP_OK) 
  {
    Serial.print(" Sent with success to: ");
    for (int j = 0; j < 6; j++) {
      Serial.printf("%02X", masterAddress[j]);
      if (j < 5) Serial.print(":");
    }
    Serial.println();
  }
  else {
    Serial.println(" Error sending the sniff data");
  }
}

void broadcastMessage(){
  int lenght = sizeof(broadcastAddresses) / sizeof(broadcastAddresses[0]); // Get the number of broadcast addresses
  for (int i = 0; i < lenght; i++)
  {
    if (i == myMacIndex) continue;

    esp_err_t result = esp_now_send(broadcastAddresses[i], (uint8_t *) &myData, sizeof(myData));
    Serial.print("Sending data to: ");
    for (int j = 0; j < 6; j++) {
      Serial.printf("%02X", broadcastAddresses[i][j]);
      if (j < 5) Serial.print(":");
    }
    if (result == ESP_OK) 
    {
      Serial.print(" Sent with success to: ");
      for (int j = 0; j < 6; j++) {
        Serial.printf("%02X", broadcastAddresses[i][j]);
        if (j < 5) Serial.print(":");
      }
      Serial.println();
    }
    else {
      Serial.println(" Error sending the data");
    }
  }
}

void setupEspNow() {
  myMacIndex = getMyIndexInList();
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  // Register peers

  int lenght = sizeof(broadcastAddresses) / sizeof(broadcastAddresses[0]);
  for (int i = 0; i < lenght; i++) {
    if (i == myMacIndex) continue; // Skip own MAC
    memcpy(peerInfo.peer_addr, broadcastAddresses[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.print("Failed to add peer: ");
        for (int j = 0; j < 6; j++) {
            Serial.printf("%02X", broadcastAddresses[i][j]);
            if (j < 5) Serial.print(":");
        }
        Serial.println();
    }
  }
}
