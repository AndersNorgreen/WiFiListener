#include <struct_message.h>
#include "sniffAndSendService.h"
#include <Arduino.h>
#include <sniffer.h>
#include <timeHandler.h>
#include <WiFi.h>
#include <espNowHandler.h>

struct_sniff_message SniffAndSendService::buildSniffMessage(const uint8_t *macAddress, const char *rssi, const char *channel, const char *timestamp) {
  struct_sniff_message sniffMsg;
  memcpy(sniffMsg.macAddress, macAddress, 6);
  strncpy(sniffMsg.RSSI, rssi, sizeof(sniffMsg.RSSI) - 1);
  sniffMsg.RSSI[sizeof(sniffMsg.RSSI) - 1] = '\0';
  strncpy(sniffMsg.channel, channel, sizeof(sniffMsg.channel) - 1);
  sniffMsg.channel[sizeof(sniffMsg.channel) - 1] = '\0';
  strncpy(sniffMsg.timestamp, timestamp, sizeof(sniffMsg.timestamp) - 1);
  sniffMsg.timestamp[sizeof(sniffMsg.timestamp) - 1] = '\0';
  WiFi.macAddress(sniffMsg.device_macAddress);
  return sniffMsg;
}

void SniffAndSendService::sniff(int channel, int millisecs) {
  SnifferService& snifferService = SnifferService::getInstance();
  snifferService.setChannel(channel);
  snifferService.setUpdateDelay(millisecs);
  snifferService.scan();
}

void SniffAndSendService::sniffCycleChannels(int millisecs) {
  SnifferService& snifferService = SnifferService::getInstance();
  snifferService.setUpdateDelay(millisecs);
  snifferService.cycleChannelsScan();
}

void SniffAndSendService::sendSniffMessages(uint8_t masterAddress[6]) {
  SnifferService& snifferService = SnifferService::getInstance();
  String (&maclist)[128][5] = snifferService.getMacList();
  int length = sizeof(maclist) / sizeof(maclist[0]);
  for (int i = 0; i < length; i++) {
    if (maclist[i][0] != "") {
      struct_sniff_message sniffMsg = buildSniffMessage(
        (const uint8_t *)maclist[i][0].c_str(),
        maclist[i][4].c_str(),
        maclist[i][3].c_str(),
        maclist[i][5].c_str());
      sendSniffMessage(sniffMsg, masterAddress);
      delay(100); // Delay to avoid flooding the network
    }
  }
}
