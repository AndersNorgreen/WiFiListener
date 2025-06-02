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

class TriangulationService {
public:
    void enableMockData(bool enable);
    void addOrUpdateDeviceTracker(const char* mac, const DevicePosition* position = nullptr);
    const std::vector<DeviceInfo>& getDevicePositions(bool clearList = true);
    void clearDevicePositions();
    void addMeasurement(char deviceMac[18], char trackerMac[18], int rssi);

private:
    struct DeviceTrackerInfo {
        char mac[18];
        bool isActive;
        long lastSeenTimestamp;
        DevicePosition position;
    };

    struct DeviceMeasurement {
        char mac[18];
        char trackerMac[18];
        int rssi;
        long timestamp;
    };

    std::vector<DeviceTrackerInfo> deviceTrackers;
    std::vector<DeviceMeasurement> deviceMeasurements;
    std::vector<DeviceInfo> verifiedDevicePositions;
    bool useMock = false;

    void addMockData();
    DeviceInfo* findOrCreateDeviceInfo(const char* mac);
    void removeDeviceInfo(const char* mac);
    void clearDeviceMeasurementsFor(const char* deviceMac);
    DevicePosition tryCalculateDevicePosition(const char* deviceMac);
};

#endif