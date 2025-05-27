#ifndef ENDPOINT_MANAGER_H
#define ENDPOINT_MANAGER_H

#include <ESPAsyncWebServer.h>

class EndpointManager {
  public:
    EndpointManager(AsyncWebServer& server);
    void serveServerApMode();
    void serveServerWpMode();
    void serveFrontEnd();
    void serveUpdateWifiConfig();
    void serveDownloadCsv();
    void serveCsvDataFile();
    void serveBackupFiles();
    void clearCsvFile();
    void createBackupFile();
  private:
    AsyncWebServer& server;
};

#endif