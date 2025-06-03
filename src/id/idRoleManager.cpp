#include <helpers.h>
#include <id/idRoleManager.h>
#include <esp_now.h>
#include <WiFi.h>
#include <id/idGenerator.h>

IdGenerator idGenerator;

struct deviceInfoTrackerInfo {
    char mac[18];
    char id[20];
    long lastSeen;
};
static deviceInfoTrackerInfo deviceInfoTracker[10];
static int deviceCount = 0;

static bool readyToCompare = false;
static unsigned long comparisonStartTime = 0;

void IdRoleManager::updateDeviceInfoTracker(const char* mac, const char* id) {
    long currentTime = millis();

    // Remove old entries
    for (int i = 0; i < deviceCount; i++) {
        if (currentTime - deviceInfoTracker[i].lastSeen > 60000) {
            for (int j = i; j < deviceCount - 1; j++) {
                deviceInfoTracker[j] = deviceInfoTracker[j + 1];
            }
            deviceCount--;
            i--;
        }
    }

    // Update or add
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(deviceInfoTracker[i].mac, mac) == 0) {
            strncpy(deviceInfoTracker[i].id, id, sizeof(deviceInfoTracker[i].id));
            deviceInfoTracker[i].lastSeen = currentTime;
            return;
        }
    }

    if (deviceCount < 10) {
        strncpy(deviceInfoTracker[deviceCount].mac, mac, sizeof(deviceInfoTracker[deviceCount].mac));
        strncpy(deviceInfoTracker[deviceCount].id, id, sizeof(deviceInfoTracker[deviceCount].id));
        deviceInfoTracker[deviceCount].lastSeen = millis();
        deviceCount++;
    } else {
        Serial.println("Device info tracker is full!");
    }
}

String macAddress = WiFi.macAddress();
char myId[20];

void IdRoleManager::sendIdTo(const uint8_t* mac) {
    struct struct_message outMsg;

    strncpy(outMsg.id, myId, sizeof(outMsg.id));
    WiFi.macAddress(outMsg.macAddress);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(mac)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer");
            return;
        }
    }

    const int maxRetries = 10;
    int retryCount = 0;
    esp_err_t result;

    delay(1000);
    do {
        result = esp_now_send(mac, (uint8_t*)&outMsg, sizeof(outMsg));
        if (result == ESP_OK) {
            Serial.print("ID sent to ");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%02X", mac[i]);
                if (i < 5) Serial.print(":");
            }
            Serial.println();
            break;
        } else {
            Serial.println("Error sending the data, retrying...");
            delay(2000); // small delay between retries
        }
    } while (++retryCount < maxRetries);

    if (result != ESP_OK) {
        Serial.println("Failed to send after retries.");
    }
}


void IdRoleManager::init() {
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb([](const uint8_t * mac, const uint8_t *data, int len) {
        static IdRoleManager self;
        self.onDataRecv(mac, data, len);
    });
}

// Managing roles and sending IDs to peers
void IdRoleManager::manageRoles() {
    idGenerator.initId();
    snprintf(myId, sizeof(myId), "%d", idGenerator.getRandomID());

    macAddress = WiFi.macAddress(); // Refresh

    Serial.println("Broadcasting my ID for discovery...");

    struct struct_message outMsg;

    strncpy(outMsg.id, myId, sizeof(outMsg.id));
    WiFi.macAddress(outMsg.macAddress);

    // Broadcast MAC address (FF:FF:FF:FF:FF:FF)
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(broadcastMac)) {
        esp_now_add_peer(&peerInfo);
    }

    for (int i = 0; i < 10; i++) {
        esp_err_t result = esp_now_send(broadcastMac, (uint8_t*)&outMsg, sizeof(outMsg));
        if (result == ESP_OK) {
            Serial.printf("Broadcasted ID to all peers (%d/10)\n", i + 1);
        } else {
            Serial.printf("Failed to broadcast ID (%d/10)\n", i + 1);
        }
        delay(1000); 
    }

    delay(1000); // Ensure ESP-NOW is ready
    Serial.printf("Device count: %d\n", deviceCount);
    for (int i = 0; i < deviceCount; i++) {
        uint8_t mac[6];
        if (sscanf(deviceInfoTracker[i].mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
            
                    Serial.print("Sending ID to: ");
                    Serial.println(deviceInfoTracker[i].mac);
                    sendIdTo(mac);
        } else {
            Serial.print("Invalid MAC string: ");
            Serial.println(deviceInfoTracker[i].mac);
        }
    }
}


// Callback function to handle incoming data
void IdRoleManager::onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    struct struct_message incomingMsg;

    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.printf("ID received from: %s - ID: %s\n", macStr, incomingMsg.id);

    updateDeviceInfoTracker(macStr, incomingMsg.id);

    //comparisonStartTime = millis();
    readyToCompare = true;
}


// Function to check if enough IDs have been received and compare roles
int IdRoleManager::checkAndCompareRoles(String& masterMacAddress) {
    if (readyToCompare) {
        if (comparisonStartTime == 0) {
            if (hasIdCollision()) {
                Serial.println("ID collision detected. Rerolling...");
                delay(4000);
                rerollAndBroadcastId();
                return -1; // Skip rest until next loop
            } else {
                Serial.println("All IDs are unique. Starting comparison timer...");
                comparisonStartTime = millis();
                return -1;
            }
        }
    
        if (readyToCompare == true && millis() - comparisonStartTime >= 5000) {
            Serial.printf("Device count: %d\n", deviceCount);
            if (deviceCount >= 1) {
                Serial.println("Enough IDs received for comparison.");

                int myIdInt = atoi(myId);
                int lowestId = myIdInt;
                String lowestMac = macAddress;

                for (int i = 0; i < deviceCount; i++) {
                    int otherId = atoi(deviceInfoTracker[i].id);
                    if (strcmp(deviceInfoTracker[i].mac, macAddress.c_str()) == 0) continue;
                    if (otherId < lowestId) {
                        lowestId = otherId;
                        lowestMac = deviceInfoTracker[i].mac;
                    }
                }

                readyToCompare = false;
                masterMacAddress = lowestMac;
                comparisonStartTime = 0;

                if (lowestId == myIdInt) {
                    Serial.println("You are the Master!");
                    return 1;
                } else {
                    Serial.println("You are NOT the Master!");
                    return 0;
                }
            }
            readyToCompare = false;
            comparisonStartTime = 0;
        }
    }
    return -1;
}

// Checks if any device ID collides with another (different MAC, same ID)
bool IdRoleManager::hasIdCollision() {
    for (int i = 0; i < deviceCount; i++) {
        for (int j = i + 1; j < deviceCount; j++) {
            if (strcmp(deviceInfoTracker[i].id, deviceInfoTracker[j].id) == 0 &&
                strcmp(deviceInfoTracker[i].mac, deviceInfoTracker[j].mac) != 0) {
                Serial.printf("Collision found between %s and %s with ID %s\n",
                              deviceInfoTracker[i].mac, deviceInfoTracker[j].mac, deviceInfoTracker[i].id);
                return true;
            }
        }
    }
    // Also check if myId collides with any other device (different MAC)
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(deviceInfoTracker[i].id, myId) == 0 &&
            strcmp(deviceInfoTracker[i].mac, macAddress.c_str()) != 0) {
            Serial.printf("My ID (%s) collides with %s\n", myId, deviceInfoTracker[i].mac);
            return true;
        }
    }
    return false;
}

// Generates a new ID, updates myId, broadcasts it to peers
void IdRoleManager::rerollAndBroadcastId() {
    idGenerator.initId();
    snprintf(myId, sizeof(myId), "%d", idGenerator.getRandomID());

    Serial.printf("Rerolled new ID: %s\n", myId);

    struct struct_message outMsg;
    strncpy(outMsg.id, myId, sizeof(outMsg.id));
    WiFi.macAddress(outMsg.macAddress);

    // Broadcast MAC address (FF:FF:FF:FF:FF:FF)
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(broadcastMac)) {
        esp_now_add_peer(&peerInfo);
    }

    for (int i = 0; i < 10; i++) {
        esp_err_t result = esp_now_send(broadcastMac, (uint8_t*)&outMsg, sizeof(outMsg));
        if (result == ESP_OK) {
            Serial.printf("Broadcasted rerolled ID to all peers (%d/10)\n", i + 1);
        } else {
            Serial.printf("Failed to broadcast rerolled ID (%d/10)\n", i + 1);
        }
        delay(1000);
    }
}