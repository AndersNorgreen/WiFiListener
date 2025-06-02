#include <server/endpointManager.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include <helpers.h>

EndpointManager::EndpointManager(AsyncWebServer& server) : server(server) {};

JsonDocument doc;

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void onRequest(AsyncWebServerRequest *request);

void onFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

void onBackupCreate(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void onCsvClear(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void onServeDownload(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void EndpointManager::serveServerApMode() {
  server.on("/api/apmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"mode\":\"AP\"}");
  });
};

void EndpointManager::serveServerWpMode() {
  server.on("/api/wpmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"mode\":\"WP\"}");
  });
}

void EndpointManager::serveFrontEnd() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", LittleFS, "/");
}

void EndpointManager::serveUpdateWifiConfig() {
  server.on("/api/setwificonfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Config has been saved, restarting...");
    Serial.println("New config received and saved, restarting...");
    delay(500);
    ESP.restart();
  }, onFileUpload, onBody);
}

void EndpointManager::serveDownloadCsv() {
  server.on("/api/downloadcsv", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Download served!");
  }, onFileUpload, onServeDownload);
}

void EndpointManager::serveCsvDataFile() {
  server.on("/api/csvdata", HTTP_GET, [&](AsyncWebServerRequest *request){
    File file = LittleFS.open(SNIFFER_DATA_PATH, FILE_READ);
    if(!file) {
      Serial.println("No temperature data file found!");
      request->send(404, "text/plain", "File not found :(");
      return;
    }

    AsyncResponseStream *response = request->beginResponseStream("text/csv");
    while (file.available()) {
      response->write(file.read());
    }

    file.close();

    request->send(response);    
  });
}

void EndpointManager::createBackupFile() {
  server.on("/api/backupcsv", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Backup created!");
  }, onFileUpload, onBackupCreate);
}

void EndpointManager::serveBackupFiles() {
 server.on("/api/getbackupfiles", HTTP_GET, [](AsyncWebServerRequest *request) {
  String response = "[";
  
  File rootPath = LittleFS.open("/backups");
  if (!rootPath) {
    Serial.println("No backup directory found!");
    request->send(404, "text/plain", "No backups found");
    return;
  }

  File file = rootPath.openNextFile();
  bool firstFile = true;

  while (file) {
    if (!file.isDirectory()) {
      String fileName = file.name();
      if (!firstFile) {
        response += ",";
      }
      response += "\"" + fileName + "\"";
      firstFile = false;
    }
  
    file = rootPath.openNextFile();
  }

  rootPath.close();

  response += "]";

  request->send(200, "application/json", response);
  });
}

void EndpointManager::clearCsvFile() {
  server.on("/api/clearcsv", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "File delete!");
  }, onFileUpload, onCsvClear);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DeserializationError error = deserializeJson(doc, (char*)data);
  if(!error) {
    const char* ssid = doc["ssid"];
    const char* password = doc["password"];
    const char* ipaddress = doc["ipaddress"];
    const char* gateway = doc["gateway"];
    Serial.println("Writing Wi-Fi config to files");
    writeFile(LittleFS, SSID_PATH, String(ssid).c_str());
    writeFile(LittleFS, PASSWORD_PATH, String(password).c_str());
    writeFile(LittleFS, IP_PATH, String(ipaddress).c_str());
    writeFile(LittleFS, GATEWAY_PATH, String(gateway).c_str());
  } else {
    Serial.println("Deserialization failed");
  }
};

void onRequest(AsyncWebServerRequest *request) {};
void onFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {};

void onBackupCreate(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DeserializationError error = deserializeJson(doc, (char*)data);
  if(!error) {
    const char* timestamp = doc["timestamp"];
    String backupPath = "/backups/backup_" + String(timestamp) + ".csv";

    copyFile(LittleFS, SNIFFER_DATA_PATH, backupPath.c_str());
  } else {
    Serial.println("Deserialization failed");
  }
}

void onCsvClear(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  DeserializationError error = deserializeJson(doc, (char*)data);
  if(!error) {
    const char* fileName = doc["fileName"];
    deleteFile(LittleFS, String(fileName).c_str());
  } else {
    Serial.println("Deserialization failed");
  }
}

void onServeDownload(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DeserializationError error = deserializeJson(doc, (char*)data);
  if(!error) {
    const char* fileName = doc["filename"];
    File file = LittleFS.open(String(fileName), FILE_READ);

    if(!file) {
      Serial.println("No temperature data file found!");
      request->send(404, "text/plain", "File not found :(");
      return;
    }

    AsyncResponseStream *response = request->beginResponseStream("text/csv");
    while (file.available()) {
      response->write(file.read());
    }

    file.close();

    request->send(response); 
  } else {
    Serial.println("Deserialization failed");
  }
}