#ifndef SNIFFER_H
#define SNIFFER_H

#include <Arduino.h>
#include <esp_wifi_types.h>

class SnifferService {
public:
  static SnifferService& getInstance();

  void setUpEspWiFi();
  void setChannel(int channel);
  void bumpChannel();
  void purge();
  void updatetime();
  void showpeople();
  void setUpdateDelay(int delay);
  void scan();
  void cycleChannelsScan();

  // Accessor for maclist
  String (&getMacList())[128][5];

private:
  SnifferService() {} // Private constructor
  SnifferService(const SnifferService&) = delete;
  SnifferService& operator=(const SnifferService&) = delete;

  static void sniffer(void* buf, wifi_promiscuous_pkt_type_t type);
  static String maclist[128][5];
  static String defaultTTL;
  static int listcount;
  static int curChannel;
  static int updateDelay;
};

#endif // SNIFFER_H
