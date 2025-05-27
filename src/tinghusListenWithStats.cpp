// #include <Arduino.h>
// #include <WiFi.h>
// #include "esp_wifi.h"
// #include <map>

// struct DeviceStats {
//     int probeCount = 0;
//     int minRssi = 127;
//     int maxRssi = -128;
//     long rssiSum = 0;

//     void update(int rssi) {
//         probeCount++;
//         if (rssi < minRssi) minRssi = rssi;
//         if (rssi > maxRssi) maxRssi = rssi;
//         rssiSum += rssi;
//     }

//     float averageRssi() const {
//         return probeCount ? (float)rssiSum / probeCount : 0;
//     }
// };

// std::map<String, DeviceStats> deviceMap;

// void printDeviceStats(const String& mac, const DeviceStats& stats) {
//     Serial.printf("Device %s: count=%d, min=%d, max=%d, avg=%.2f\n",
//         mac.c_str(), stats.probeCount, stats.minRssi, stats.maxRssi, stats.averageRssi());
// }

// void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
//     if (type != WIFI_PKT_MGMT) return;

//     const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
//     const uint8_t *payload = ppkt->payload;

//     if ((payload[0] & 0xF0) == 0x40) {
//         char macStr[18];
//         sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
//                 payload[10], payload[11], payload[12],
//                 payload[13], payload[14], payload[15]);
//         int rssi = ppkt->rx_ctrl.rssi;

//         String mac(macStr);
//         deviceMap[mac].update(rssi);
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     WiFi.mode(WIFI_MODE_STA);
//     esp_wifi_set_promiscuous(true);
//     esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
//     Serial.println("Listening for WiFi probe requests...");
// }

// void loop() {
//     static uint8_t channel = 1;
//     esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
//     channel++;
//     if (channel > 13) channel = 1;

//     // Print stats every 5 seconds
//     static unsigned long lastPrint = 0;
//     if (millis() - lastPrint > 5000) {
//         Serial.println("--- Device Stats ---");
//         for (const auto& entry : deviceMap) {
//             printDeviceStats(entry.first, entry.second);
//         }
//         Serial.println("--------------------");
//         lastPrint = millis();
//     }
//     delay(1000);
// }