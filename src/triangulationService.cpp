#include <string.h>
#include <esp32-hal.h>
#include <vector>
#include <Arduino.h>


struct DevicePosition {
    int x;
    int y;
};

struct DeviceInfo {
    char mac[18]; // Avoid pushing mac to server, use it only for local processing
    DevicePosition position;
};

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

const int MAX_TIME_DIFF_SECONDS = 10; // Maximum time difference in seconds to consider measurements valid.
const int MAX_TRACKING_LIFETIME_SECONDS = 60; // Maximum lifetime of a tracking measurement in seconds.
const int TIME_BEFORE_TRACKER_DEACTIVATION_SECONDS = 120; // Time before a tracker is considered inactive.
const int TIME_BEFORE_TRACKER_IS_DELETED_MINUTES = 60; // Time before a tracker is deleted from the system.
const int MIN_REQUIRED_UNIQUE_TRACKERs = 3; // Minimum number of unique trackers required to consider triangulation valid.

std::vector<DeviceTrackerInfo> deviceTrackers;
std::vector<DeviceMeasurement> deviceMeasurements;

void addOrUpdateDeviceTracker(const char* mac, const DevicePosition* position = nullptr) {
    for (auto& tracker : deviceTrackers) {
        if (strcmp(tracker.mac, mac) == 0) {
            // Tracker exists: update timestamp and optionally position
            tracker.lastSeenTimestamp = millis();
            tracker.isActive = true;
            if (position != nullptr) {
                tracker.position = *position;
            }
            return;
        }
    }

    // Tracker does not exist
    if (position == nullptr) {
        Serial.printf("Error: Cannot add tracker %s without position!\n", mac);
        return;
    }
    DeviceTrackerInfo newTracker;
    strncpy(newTracker.mac, mac, sizeof(newTracker.mac));
    newTracker.mac[sizeof(newTracker.mac) - 1] = '\0';
    newTracker.isActive = true;
    newTracker.lastSeenTimestamp = millis();
    newTracker.position = *position;
    deviceTrackers.push_back(newTracker);
}

const std::vector<DeviceTrackerInfo>& getAllDeviceTrackers() {
    return deviceTrackers;
}

void addMeasurement(char deviceMac[18], char trackerMac[18], int rssi) {
    long currentTime = millis();

    // Add new measurement
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
            [currentTime](const DeviceMeasurement& measurement) {
                return (currentTime - measurement.timestamp) > MAX_TRACKING_LIFETIME_SECONDS * 1000;
            }
        ),
        deviceMeasurements.end()
    );

    tryCalculateDevicePosition(deviceMac); // TODO: implement this
}


DevicePosition tryCalculateDevicePosition(const char* deviceMac) {
    std::vector<DeviceMeasurement> relevantMeasurements;
    long currentTime = millis();

    for (const auto& measurement : deviceMeasurements) {
        if (strcmp(measurement.mac, deviceMac) == 0 &&
            (currentTime - measurement.timestamp) <= MAX_TIME_DIFF_SECONDS * 1000) {
            relevantMeasurements.push_back(measurement);
        }
    }

    if (relevantMeasurements.size() < MIN_REQUIRED_UNIQUE_TRACKERs) {
        Serial.printf("Not enough measurements to calculate position for device %s\n", deviceMac);
        return;
    }

    DevicePosition calculatedPosition;
    calculatedPosition.x = 0; // TODO: Replace with calculation logic
    calculatedPosition.y = 0; // TODO: Replace with calculation logic

    return calculatedPosition;
}