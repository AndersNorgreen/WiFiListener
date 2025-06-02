#include "triangulationService.h"
#include <string.h>
#include <esp32-hal.h> // For millis() to ensure compatibility with ESP32 we do not use Arduino.h directly
#include <vector>
#include <Arduino.h>
#include <algorithm>

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

const int MAX_TIME_DIFF_SECONDS = 10;
const int MAX_TRACKING_LIFETIME_SECONDS = 60;
const int MIN_REQUIRED_UNIQUE_TRACKERS = 3;

static std::vector<DeviceTrackerInfo> deviceTrackers;
static std::vector<DeviceMeasurement> deviceMeasurements;
static std::vector<DeviceInfo> verifiedDevicePositions;
static bool useMock = false;

static void addMockData() {
    deviceTrackers.clear();
    deviceMeasurements.clear();

    DeviceTrackerInfo t1 = {"AA:BB:CC:DD:EE:01", true, millis(), {0, 0}};
    DeviceTrackerInfo t2 = {"AA:BB:CC:DD:EE:02", true, millis(), {100, 0}};
    DeviceTrackerInfo t3 = {"AA:BB:CC:DD:EE:03", true, millis(), {50, 86}};
    deviceTrackers.push_back(t1);
    deviceTrackers.push_back(t2);
    deviceTrackers.push_back(t3);

    // Simulate a device
    DeviceMeasurement m1 = {"11:22:33:44:55:66", "AA:BB:CC:DD:EE:01", -40, millis()};
    DeviceMeasurement m2 = {"11:22:33:44:55:66", "AA:BB:CC:DD:EE:02", -45, millis()};
    DeviceMeasurement m3 = {"11:22:33:44:55:66", "AA:BB:CC:DD:EE:03", -50, millis()};
    deviceMeasurements.push_back(m1);
    deviceMeasurements.push_back(m2);
    deviceMeasurements.push_back(m3);
}

void enableMockData(bool enable) {
    useMock = enable;
    if (useMock) {
        addMockData();
        Serial.println("[MOCK] Mock data enabled.");
    } else {
        Serial.println("[MOCK] Mock data disabled.");
    }
}

void addOrUpdateDeviceTracker(const char* mac, const DevicePosition* position) {
    for (auto& tracker : deviceTrackers) {
        if (strcmp(tracker.mac, mac) == 0) {
            tracker.lastSeenTimestamp = millis();
            tracker.isActive = true;
            if (position) tracker.position = *position;
            Serial.printf("[TRACKER] Updated tracker %s\n", mac);
            return;
        }
    }
    if (!position) {
        Serial.printf("[TRACKER] Error: Cannot add tracker %s without position!\n", mac);
        return;
    }
    DeviceTrackerInfo newTracker;
    strncpy(newTracker.mac, mac, sizeof(newTracker.mac));
    newTracker.mac[sizeof(newTracker.mac) - 1] = '\0';
    newTracker.isActive = true;
    newTracker.lastSeenTimestamp = millis();
    newTracker.position = *position;
    deviceTrackers.push_back(newTracker);
    Serial.printf("[TRACKER] Added new tracker %s\n", mac);
}

static DeviceInfo* findOrCreateDeviceInfo(const char* mac) {
    for (auto& info : verifiedDevicePositions) {
        if (strcmp(info.mac, mac) == 0) return &info;
    }
    DeviceInfo info;
    strncpy(info.mac, mac, sizeof(info.mac));
    info.mac[sizeof(info.mac) - 1] = '\0';
    info.position = {0, 0};
    verifiedDevicePositions.push_back(info);
    return &verifiedDevicePositions.back();
}

static void removeDeviceInfo(const char* mac) {
    verifiedDevicePositions.erase(
        std::remove_if(
            verifiedDevicePositions.begin(),
            verifiedDevicePositions.end(),
            [mac](const DeviceInfo& info) { return strcmp(info.mac, mac) == 0; }
        ),
        verifiedDevicePositions.end()
    );
}

static void clearDeviceMeasurementsFor(const char* deviceMac) {
    deviceMeasurements.erase(
        std::remove_if(
            deviceMeasurements.begin(),
            deviceMeasurements.end(),
            [deviceMac](const DeviceMeasurement& m) {
                return strcmp(m.mac, deviceMac) == 0;
            }
        ),
        deviceMeasurements.end()
    );
}

static DevicePosition tryCalculateDevicePosition(const char* deviceMac) {
    long now = millis();
    std::vector<DeviceMeasurement> relevant;
    std::vector<std::string> uniqueTrackers;

    // Gather recent measurements for device
    for (const auto& m : deviceMeasurements) {
        if (strcmp(m.mac, deviceMac) == 0 && (now - m.timestamp) <= MAX_TIME_DIFF_SECONDS * 1000) {
            relevant.push_back(m);
            if (std::find(uniqueTrackers.begin(), uniqueTrackers.end(), m.trackerMac) == uniqueTrackers.end())
                uniqueTrackers.push_back(m.trackerMac);
        }
    }

    if (uniqueTrackers.size() < MIN_REQUIRED_UNIQUE_TRACKERS) {
        Serial.printf("[TRIANGULATION] Not enough unique trackers for device %s (%d found)\n", deviceMac, (int)uniqueTrackers.size());
        return {0, 0}; // Not enough data
    }

    // Weighted centroid based on RSSI
    float sumX = 0, sumY = 0, sumWeight = 0;
    for (const auto& m : relevant) {
        auto it = std::find_if(deviceTrackers.begin(), deviceTrackers.end(),
            [&](const DeviceTrackerInfo& t) { return strcmp(t.mac, m.trackerMac) == 0; });
        if (it != deviceTrackers.end()) {
            float weight = 127 + m.rssi;
            if (weight < 1) weight = 1;
            sumX += it->position.x * weight;
            sumY += it->position.y * weight;
            sumWeight += weight;
        }
    }
    if (sumWeight == 0) {
        Serial.printf("[TRIANGULATION] No valid tracker positions for device %s\n", deviceMac);
        return {0, 0};
    }
    DevicePosition pos = {
        static_cast<int>(sumX / sumWeight),
        static_cast<int>(sumY / sumWeight)
    };
    Serial.printf("[TRIANGULATION] Device %s position calculated: (%d, %d)\n", deviceMac, pos.x, pos.y);
    return pos;
}

void addMeasurement(char deviceMac[18], char trackerMac[18], int rssi) {
    long currentTime = millis();
    DeviceMeasurement newMeasurement;
    strncpy(newMeasurement.mac, deviceMac, sizeof(newMeasurement.mac));
    newMeasurement.mac[sizeof(newMeasurement.mac) - 1] = '\0';
    strncpy(newMeasurement.trackerMac, trackerMac, sizeof(newMeasurement.trackerMac));
    newMeasurement.trackerMac[sizeof(newMeasurement.trackerMac) - 1] = '\0';
    newMeasurement.rssi = rssi;
    newMeasurement.timestamp = currentTime;
    deviceMeasurements.push_back(newMeasurement);

    // Clean up old measurements
    deviceMeasurements.erase(
        std::remove_if(
            deviceMeasurements.begin(),
            deviceMeasurements.end(),
            [currentTime](const DeviceMeasurement& m) {
                return (currentTime - m.timestamp) > MAX_TRACKING_LIFETIME_SECONDS * 1000;
            }
        ),
        deviceMeasurements.end()
    );
    Serial.printf("Added measurement for %s from tracker %s (RSSI: %d)\n", deviceMac, trackerMac, rssi);

    // Try to calculate position for device
    DevicePosition pos = tryCalculateDevicePosition(deviceMac);
    if (pos.x != 0 || pos.y != 0) {
        DeviceInfo* info = findOrCreateDeviceInfo(deviceMac);
        info->position = pos;
        Serial.printf("Updated/added device %s at (%d, %d)\n", deviceMac, pos.x, pos.y);
        clearDeviceMeasurementsFor(deviceMac);  // Clear measurements for device to avoid duplicates
    }
}

const std::vector<DeviceInfo>& getDevicePositions(bool clearList) {
    if (useMock) {
        addMockData();
        // Recalculate all mock device positions
        for (const auto& m : deviceMeasurements) {
            tryCalculateDevicePosition(m.mac);
        }
    }
    if (clearList) {
        static std::vector<DeviceInfo> result;
        result = verifiedDevicePositions;
        verifiedDevicePositions.clear();
        return result;
    }
    return verifiedDevicePositions;
}

void clearDevicePositions() {
    verifiedDevicePositions.clear();
    Serial.println("Cleared all verified device positions.");
}