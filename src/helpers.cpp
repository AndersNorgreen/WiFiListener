#include <helpers.h>
#include <iostream>
#include <filesystem>
#include <Arduino.h>
#include "LittleFS.h"
#include <WiFi.h>
#include <server/wifiConfig.h>
#include "time.h"
#include "config.h"

using namespace std;

timeInitializationData initTimeData;

void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

String generateGuid() {
    char guid[37]; // UUID format: 8-4-4-4-12
    const char *hex_chars = "0123456789ABCDEF";

    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            guid[i] = '-';
        } else {
            guid[i] = hex_chars[random(0, 16)];
        }
    }
    guid[36] = '\0';

    return String(guid);
}

String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }

  return fileContent;
}

void writeFile(fs::FS &fs, const char* path, const char* message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);

  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }

  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }

  file.close();
}

void copyFile(fs::FS &fs, const char * sourcePath, const char * destinationPath) {
  createDirectoryIfNeeded(LittleFS, destinationPath);

  if (!LittleFS.exists(sourcePath)) {
    Serial.println("The source file for copying could not be found!");
    return;
  }

  File sourceFile = LittleFS.open(sourcePath, FILE_READ);
  if (!sourceFile) {
    Serial.println("Failed to open the source file for reading.");
    return;
  }

  File destinationFile = LittleFS.open(destinationPath, FILE_WRITE);
  while(sourceFile.available()) {
    destinationFile.write(sourceFile.read());
  }

  Serial.println("File copied to " + String(destinationPath) + "!");

  sourceFile.close();
  destinationFile.close();
}

void deleteFile(fs::FS &fs, const char * filePath) {
  if(!fs.exists(filePath)) {
    Serial.println("No file found for deletion!");
    return;
  }

  fs.remove(filePath);
  Serial.println("Requested file deleted!");
}

void updateFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
      Serial.println("- failed to open file for appending");
      return;
  }

  if(file.print(message)){
      Serial.println("- message appended");
  } else {
      Serial.println("- append failed");
  }

  file.close(); 
}

void initTime() {
  WiFi.begin(CURRENT_SSID, CURRENT_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  configTime(initTimeData.gmtOffset_sec, initTimeData.daylightOffset_sec, initTimeData.ntpServer);
  if(!getLocalTime(&initTimeData.timeInfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void createDirectoryIfNeeded(fs::FS &fs, const char *path) {
  String pathStr = String(path);
  int lastSlash = pathStr.lastIndexOf('/');

  if (lastSlash != -1) {
    String dirPath = pathStr.substring(0, lastSlash);

    if (!fs.exists(dirPath)) {
      if (fs.mkdir(dirPath)) {
        Serial.println("Directory created: " + dirPath);
      } else {
        Serial.println("Failed to create directory: " + dirPath);
      }
    }
  }
}

void listFiles(const char* dir) {
  String path = dir;
  if (path[0] != '/') {
    path = "/" + path;
  }

  File root = LittleFS.open(path);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  if (root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.print("Dir: ");
        Serial.println(file.name());
        listFiles(file.name());
      } else {
        Serial.print("File: ");
        Serial.println(file.name());
      }
      file = root.openNextFile();
    }
  }
  root.close();
}