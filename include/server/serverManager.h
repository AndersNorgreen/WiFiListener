#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <server/wifiConfig.h>
#include <server/endpointManager.h>
#include "config.h"

class ServerManager {
  public:
    ServerManager();
    void initServer();
    bool initWifi();
    void initWifiServer();
    void initAPServer();
    void updateWifiConfig(const WifiConfig& newConfig);
    void clearWifiConfig(const int buttonState);

    AsyncWebServer& getServer() { return server; }
  private:
    AsyncWebServer server;
    EndpointManager endpoints;
    WifiConfig config;
};

#endif