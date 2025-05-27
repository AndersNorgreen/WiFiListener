#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h> 
#include "LittleFS.h"
#include <wifiConfig.h>

void writeFile(fs::FS &fs, const char * path, const char * message);
void updateFile(fs::FS &fs, const char * path, const char * message);
void copyFile(fs::FS &fs, const char * sourcePath, const char * destinationPath);
void deleteFile(fs::FS &fs, const char * filePath);
void createDirectoryIfNeeded(fs::FS &fs, const char *path);
void initTime();
void initLittleFS();
void listFiles(const char* dir);

String readFile(fs::FS &fs, const char * path);

struct timeInitializationData {
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;
  struct tm timeInfo;
};
extern timeInitializationData initTimeData;

#endif