#include <server/serverManager.h>

ServerManager::ServerManager() : server(80), endpoints(server), config() {};

void ServerManager::initServer() {
  if(!initWifi()) {   
    initAPServer();
  } else {
    initWifiServer();
  };
}

bool ServerManager::initWifi()
{
  if(config.getSSID() == "" || config.getPassword() == "") {
    Serial.println("No SSID or IP address found!");
    return false;
  }

  WiFi.mode(WIFI_STA);
  if(!WiFi.config(config.getIpAddress(), config.getGateway(), config.getSubnet())) {
    Serial.println("STA Failed to configure");
    return false;
  }

  WiFi.begin(config.getSSID(), config.getPassword());
  Serial.println("Connecting to WiFi");

  config.setCurrentMillis();
  config.setPreviousMillis(config.getCurrentMillis());

  while(WiFi.status() != WL_CONNECTED) {
    config.setCurrentMillis();
    if (config.getCurrentMillis() - config.getPreviousMillis() >= config.getInterval()) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  return true;
}

void ServerManager::initAPServer() {
  Serial.println("Setting AP (Access Point)");

  WiFi.softAP("MANGO-WIFI-MANAGER", "MANGOFEST");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP); 

  endpoints.serveServerApMode();
  endpoints.serveFrontEnd();
  endpoints.serveUpdateWifiConfig();

  server.begin();
}

void ServerManager::initWifiServer() {
  endpoints.serveServerWpMode();
  endpoints.serveFrontEnd();
  endpoints.serveDownloadCsv();
  endpoints.serveCsvDataFile();
  endpoints.serveBackupFiles();
  endpoints.createBackupFile();
  endpoints.clearCsvFile();
  
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");

  server.begin();
  
  Serial.print("Connect to IP Address: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void ServerManager::updateWifiConfig(const WifiConfig& newConfig) {
  config = newConfig;
}

void ServerManager::clearWifiConfig(const int buttonState) {
  Serial.println("Removing WiFi config...");
  LittleFS.remove(SSID_PATH);
  LittleFS.remove(PASSWORD_PATH);
  LittleFS.remove(IP_PATH);
  LittleFS.remove(GATEWAY_PATH);

  WifiConfig emptyConfig;
  updateWifiConfig(emptyConfig);

  Serial.println("Config files erased, restarting in five...");
  delay(5000);

  initServer();
}