#include "sniffer.h"
#include <Arduino.h>
#include <WiFi.h>
#include <formatHandler.h>
#include <esp_wifi.h>
#include <struct_message.h>

// String maclist[128][5]; // 0 = MAC address, 1 = TTL, 2 = ONLINE/OFFLINE, 3 = Channel, 4 = RSSI, 5 = Timestamp
// String defaultTTL = "60"; // Maximum time (Apx seconds) elapsed before device is consirded offline
// int listcount = 0;
// int curChannel = 1;
// int updateDelay = 1000; // How long to wait before updating the list of devices

#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14
String SnifferService::maclist[128][5]; // 0 = MAC address, 1 = TTL, 2 = ONLINE/OFFLINE, 3 = Channel, 4 = RSSI, 5 = Timestamp
String SnifferService::defaultTTL = "60"; // Maximum time (Apx seconds) elapsed before device is consirded offline
int SnifferService::listcount = 0;
int SnifferService::curChannel = 1;
int SnifferService::updateDelay = 1000; // How long to wait before updating the list of devices

SnifferService& SnifferService::getInstance() {
    static SnifferService instance;
    return instance;
}

//This should be its own thing.
String KnownMac[10][2] = {  // Put devices you want to be reconized
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"},
  {"Device-Name","MAC-Address"}
};

const wifi_promiscuous_filter_t filt={ //Idk what this does
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};

typedef struct { // or this
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct { // still dont know much about this
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

void SnifferService::sniffer(void* buf, wifi_promiscuous_pkt_type_t type) { //This is where packets end up after they get sniffed
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf; // Dont know what these 3 lines do
  int len = p->rx_ctrl.sig_len;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  len -= sizeof(WifiMgmtHdr);
  if (len < 0){
    Serial.println("Receuved 0");
    return;
  }
  String packet;
  String mac;
  int fctl = ntohs(wh->fctl);
  for(int i=8;i<=p->rx_ctrl.sig_len;i++){ // This reads the first couple of bytes of the packet. This is where you can read the whole packet replaceing the "8+6+1" with "p->rx_ctrl.sig_len"
     packet += String(p->payload[i],HEX);
    }
  //Serial.println(packet);
  for(int i=4;i<=14;i++){ // This removes the 'nibble' bits from the stat and end of the data we want. So we only get the mac address.
    mac += packet[i];
  }
  mac.toUpperCase();

  // trying to sniff wifi frames
  // int frameType = (fctl & 0x0C) >> 2;
  // int frameSubType = (fctl & 0xF0) >> 4;
  // if (frameType == 0 && (frameSubType == 8 || frameSubType == 4 || frameSubType == 5)) {
  //   String ssid = parseSSID(p->payload, p->rx_ctrl.sig_len, frameSubType);
  //   Serial.println("SSID: " + ssid);
  // }

  int added = 0;
  for(int i=0;i<=127;i++){ // checks if the MAC address has been added before
    if(mac == maclist[i][0]){
      maclist[i][1] = defaultTTL;
      if(maclist[i][2] == "OFFLINE"){
        maclist[i][2] = "0";
      }
      added = 1;
    }
  }

  if(added == 0){
    maclist[listcount][0] = mac;
    maclist[listcount][1] = defaultTTL;
    int rssi = p->rx_ctrl.rssi;
    String timestamp = String(p->rx_ctrl.timestamp);
    Serial.print("MAC: ");
    Serial.print(mac);
    Serial.print(" RSSI: ");
    Serial.print(rssi);
    Serial.print(" Timestamp: ");
    Serial.println(timestamp);
    
    maclist[listcount][3] = String(curChannel); // This is the channel the device was found on
    maclist[listcount][4] = String(rssi); // This is the RSSI of the device
    maclist[listcount][5] = timestamp; // This is the timestamp of when the device was found


    //   WifiMgmtHdr *hdr = (WifiMgmtHdr*)p->payload;
    //   String srcMac = String((char*)hdr->sa.mac, 6);
    //   String dstMac = String((char*)hdr->da.mac, 6);
    //   String bssid  = String((char*)hdr->bssid.mac, 6);
    //   Serial.println("Source MAC: " + srcMac);
    //   Serial.println("Destination MAC: " + dstMac);
    //   Serial.println("BSSID: " + bssid);
    
    listcount ++;
    if(listcount >= 128){
      Serial.println("Too many addresses");
      listcount = 0;
    }
  }
}

String (&SnifferService::getMacList())[128][5] {
    return maclist;
}

void SnifferService::setUpEspWiFi(){
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
}

void SnifferService::setChannel(int channel) {
  if (channel < 1 || channel > maxCh) {
    Serial.println("Invalid channel. Please choose a channel between 1 and " + String(maxCh));
    return;
  }
  curChannel = channel;
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
  Serial.println("Switched to channel: " + String(curChannel));
}

void SnifferService::bumpChannel() {
  curChannel++;
  if (curChannel > maxCh) {
    curChannel = 1; // Wrap around to channel 1
  }
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
  Serial.println("Bumped to channel: " + String(curChannel));
}

void SnifferService::purge(){ // This maanges the TTL
  for(int i=0;i<=127;i++){
    if(!(maclist[i][0] == "")){
      int ttl = (maclist[i][1].toInt());
      ttl --;
      if(ttl <= 0){
        Serial.println("OFFLINE: " + maclist[i][0]);
        maclist[i][2] = "OFFLINE";
        maclist[i][1] = defaultTTL;
      }else{
        maclist[i][1] = String(ttl);
      }
    }
  }
}

void SnifferService::updatetime(){ // This updates the time the device has been online for
  for(int i=0;i<=127;i++){
    if(!(maclist[i][0] == "")){
      if(maclist[i][2] == "")maclist[i][2] = "0";
      if(!(maclist[i][2] == "OFFLINE")){
          int timehere = (maclist[i][2].toInt());
          timehere ++;
          maclist[i][2] = String(timehere);
      }
      
      //Serial.println(formatMac(maclist[i][0]) + " : " + maclist[i][2]);
      
    }
  }
}

void SnifferService::showpeople(){ // This checks if the MAC is in the reckonized list and then displays it on the OLED and/or prints it to serial.
  for(int i=0;i<=127;i++){
    String tmp1 = maclist[i][0];
    if(!(tmp1 == "")){
      for(int j=0;j<=9;j++){
        String tmp2 = KnownMac[j][1];
        String tmp3 = KnownMac[j][0];
        //Serial.println("Checking: " + tmp3 + ":" + tmp2 + " against " + tmp1);
        if(tmp1 == tmp2){
          Serial.print(KnownMac[j][0] + " : " + tmp1 + " : " + maclist[i][2] + "\n -- \n");
        }
      }
    }
  }
}

void SnifferService::setUpdateDelay(int delay) { // This sets the delay between updates
  if (delay < 100) {
    Serial.println("Update delay too short, setting to 100ms");
    updateDelay = 100;
  } else {
    updateDelay = delay;
  }
  Serial.println("Update delay set to: " + String(updateDelay) + "ms");
}

void SnifferService::scan() { // This scans for a given amount of time
  updatetime();
  showpeople();
  purge();
  delay(updateDelay); // Wait a second before starting the scan
  // unsigned long startTime = millis();
  // while (millis() - startTime < millisecs) {
  //   delay(100); // Adjust the delay as needed
  //   updatetime();
  //   purge();
  //   showpeople();
  // }
}

void  SnifferService::cycleChannelsScan() { // This scans for a given amount of time on each channel
  scan();
  bumpChannel();
}

