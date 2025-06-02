#ifndef TRIANGULATION_SERVICE_H
#define TRIANGULATION_SERVICE_H
#include <vector>

struct DevicePosition {
    int x;
    int y;
};

struct DeviceInfo {
    char mac[18];
    DevicePosition position;
};

void addOrUpdateDeviceTracker(const char* mac, const DevicePosition* position = nullptr);
const std::vector<DeviceInfo>& getDevicePositions(bool clearList = true);
void clearDevicePositions();
void addMeasurement(char deviceMac[18], char trackerMac[18], int rssi);
void enableMockData(bool enable);

#endif