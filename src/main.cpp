/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <WiFi.h>
#include "blinkLED.h"
#include "timeHandler.h"


const char* ssid = "IoT_H3/4";
const char* password = "98806829";

const char* ntpServer = "0.dk.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

WiFiClient espClient;
//PubSubClient client(espClient);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x1C ,0xA8, 0x6C}; // esp 04
uint8_t broadcastAddress1[] = {0xCC, 0xDB, 0xA7, 0x12 ,0x51, 0x0C}; // esp 01
uint8_t broadcastAddress2[] = {0xCC, 0xDB, 0xA7, 0x1E ,0x1F, 0x18}; // esp 02


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char time[20];
  uint8_t macAddress[6];
} struct_message;

// Create a struct_message called myData
struct_message myData;
struct_message incomingReadings;

// Variables to hold incoming data
String incomingTime;
uint8_t incomingMacAddress[6];


esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTime = String(incomingReadings.time);
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
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Connect to WiFi
  //client.setServer(mqtt_server, 1883);
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
  
  // Set up ntp server for time synchronization
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  // Set values to send
  strcpy(myData.time, getTimeString().c_str());
  // Send the MAC address of the wifi router
  memcpy(myData.macAddress, WiFi.BSSID(), 6);

  // Print values to Serial Monitor
  // Serial.print("Date: ");
  // printLocalTime();
  
  // Send message via ESP-NOW
  //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  // print my data
  // Serial.print("Time: ");
  // Serial.println(myData.time);
  // Serial.print("MAC Address: ");
  // for (int i = 0; i < 6; i++) {
  //   Serial.printf("%02X", myData.macAddress[i]);
  //   if (i < 5) Serial.print(":");
  // }
  // Serial.println();
  // Serial.print("Size of myData: ");
  // Serial.println(sizeof(myData));
   
  // if (result == ESP_OK) {
  //   Serial.println("Sent with success");
  // }
  // else {
  //   Serial.println("Error sending the data");
  // }
  delay(2000);
}