#include <helpers.h>
#include <id/idRoleManager.h>
#include <esp_now.h>
#include <WiFi.h>
#include <id/idGenerator.h>

IdGenerator idGenerator;

uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x1C, 0xA8, 0x6C};  // Michael
uint8_t broadcastAddress1[] = {0xCC, 0xDB, 0xA7, 0x12, 0x51, 0x0C}; // Anders
uint8_t broadcastAddress2[] = {0xCC, 0xDB, 0xA7, 0x1E, 0x1F, 0x18}; // Afshin
uint8_t broadcastAddress3[] = {0xCC, 0xDB, 0xA7, 0x1D, 0xFD, 0xFC}; // Søren
uint8_t broadcastAddress4[] = {0xEC, 0x64, 0xC9, 0x85, 0xBF, 0xB8}; // Test Address

String macAddress = WiFi.macAddress();
char myId[20];

bool IdRoleManager::compareMacArrays(const uint8_t *arr, const String &macStr)
{
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
    return macStr.equalsIgnoreCase(buf);
}

void IdRoleManager::blankMac(uint8_t *macArr)
{
    for (int i = 0; i < 6; i++)
    {
        macArr[i] = 0x00;
    }
}

void IdRoleManager::sendIdTo(const uint8_t *mac)
{
    struct struct_message
    {
        char id[20];
        uint8_t macAddress[6];
    } outMsg;

    strncpy(outMsg.id, myId, sizeof(outMsg.id));
    WiFi.macAddress(outMsg.macAddress);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(mac))
    {
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer");
            return;
        }
    }

    const int maxRetries = 10;
    int retryCount = 0;
    esp_err_t result;

    delay(5000);

    do
    {
        result = esp_now_send(mac, (uint8_t *)&outMsg, sizeof(outMsg));
        if (result == ESP_OK)
        {
            Serial.print("ID sent to ");
            for (int i = 0; i < 6; i++)
            {
                Serial.printf("%02X", mac[i]);
                if (i < 5)
                    Serial.print(":");
            }
            Serial.println();
            break;
        }
        else
        {
            Serial.println("Error sending the data, retrying...");
            delay(2000); // small delay between retries
        }
    } while (++retryCount < maxRetries);

    if (result != ESP_OK)
    {
        Serial.println("Failed to send after retries.");
    }
}

void IdRoleManager::init()
{
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len)
                             {
                                 static IdRoleManager *self = nullptr;
                                 if (!self)
                                     self = new IdRoleManager();
                                 self->onDataRecv(mac, data, len);
                                 // self->onTestDataRecv(mac, data, len);
                             });
}

// Callback function to handle incoming data
void IdRoleManager::onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    struct struct_message
    {
        char id[20];
        uint8_t macAddress[6];
    } incomingMsg;

    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

    Serial.print("ID received from: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", mac[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.print(" - ID: ");
    Serial.println(incomingMsg.id);

    // Check which peer this is and mark as received
    if (memcmp(mac, broadcastAddress, 6) == 0 && !receivedFrom[0])
    {
        receivedFrom[0] = true;
        receivedCount++;
        strncpy(receivedIds[0], incomingMsg.id, sizeof(receivedIds[0]));
    }
    else if (memcmp(mac, broadcastAddress1, 6) == 0 && !receivedFrom[1])
    {
        receivedFrom[1] = true;
        receivedCount++;
        strncpy(receivedIds[1], incomingMsg.id, sizeof(receivedIds[1]));
    }
    else if (memcmp(mac, broadcastAddress2, 6) == 0 && !receivedFrom[2])
    {
        receivedFrom[2] = true;
        receivedCount++;
        strncpy(receivedIds[2], incomingMsg.id, sizeof(receivedIds[2]));
    }
    else if (memcmp(mac, broadcastAddress3, 6) == 0 && !receivedFrom[3])
    {
        receivedFrom[3] = true;
        receivedCount++;
        strncpy(receivedIds[3], incomingMsg.id, sizeof(receivedIds[3]));
    }

    if (receivedCount == 3)
    {
        Serial.println("All IDs received!");

        int myIdInt = atoi(myId);
        bool amMaster = true;

        for (int i = 0; i < 4; i++)
        {
            if (receivedFrom[i])
            {
                int otherId = atoi(receivedIds[i]);
                if (myIdInt >= otherId)
                {
                    amMaster = false;
                    break;
                }
            }
        }

        if (amMaster)
        {
            Serial.println("You are the Master!");
        }
        else
        {
            Serial.println("You are NOT the Master!");
        }
    }
}

// Managing roles and sending IDs to peers
void IdRoleManager::manageRoles()
{
    idGenerator.initId();
    snprintf(myId, sizeof(myId), "%d", idGenerator.getRandomID());

    macAddress = WiFi.macAddress(); // Refresh

    if (compareMacArrays(broadcastAddress, macAddress))
    {
        blankMac(broadcastAddress);
        Serial.println("BroadcastAddress matched own MAC and was blanked.");
    }
    if (compareMacArrays(broadcastAddress1, macAddress))
    {
        blankMac(broadcastAddress1);
        Serial.println("BroadcastAddress1 matched own MAC and was blanked.");
    }
    if (compareMacArrays(broadcastAddress2, macAddress))
    {
        blankMac(broadcastAddress2);
        Serial.println("BroadcastAddress2 matched own MAC and was blanked.");
    }
    if (compareMacArrays(broadcastAddress3, macAddress))
    {
        blankMac(broadcastAddress3);
        Serial.println("BroadcastAddress3 matched own MAC and was blanked.");
    }

    // Send ID to all non-blank addresses
    delay(1000); // Ensure ESP-NOW is ready
    if (broadcastAddress[0] != 0x00)
        sendIdTo(broadcastAddress);
    if (broadcastAddress1[0] != 0x00)
        sendIdTo(broadcastAddress1);
    if (broadcastAddress2[0] != 0x00)
        sendIdTo(broadcastAddress2);
    if (broadcastAddress3[0] != 0x00)
        sendIdTo(broadcastAddress3);
}

// #############################################################################

// Test functions for managing roles
void IdRoleManager::manageTestRoles()
{
    idGenerator.initId();
    snprintf(myId, sizeof(myId), "%d", idGenerator.getRandomID());

    macAddress = WiFi.macAddress(); // Refresh

    if (compareMacArrays(broadcastAddress2, macAddress))
    {
        blankMac(broadcastAddress2);
        Serial.println("Test: broadcastAddress2 matched own MAC and was blanked.");
    }
    if (compareMacArrays(broadcastAddress4, macAddress))
    {
        blankMac(broadcastAddress4);
        Serial.println("Test: broadcastAddress4 matched own MAC and was blanked.");
    }

    delay(1000); // Ensure ESP-NOW is ready
    if (broadcastAddress2[0] != 0x00)
        sendIdTo(broadcastAddress2);
    if (broadcastAddress4[0] != 0x00)
        sendIdTo(broadcastAddress4);
}

void IdRoleManager::testInit()
{
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len)
                             {
        static IdRoleManager* self = nullptr;
        if (!self) self = new IdRoleManager();
        //self->onDataRecv(mac, data, len);
        self->onTestDataRecv(mac, data, len); });
}

void IdRoleManager::updateDeviceList(char trackerMac[18], int deviceId)
{
    String myMac = WiFi.macAddress();
    long currentTime = millis();
    
    // If trackerMac is my mac address, ensure unique ID and update deviceId with myId
    if (myMac.equalsIgnoreCase(trackerMac))
    {
        ensureUniqueId(deviceId);
        deviceId = myId;
        return;
    }

    // remove trackers that have not been seen for more than 10 minutes
    deviceTrackers.erase(
        std::remove_if(deviceTrackers.begin(), deviceTrackers.end(),
                       [currentTime](const deviceTracker &tracker) {
                           return (currentTime - tracker.lastSeenTimestamp) > 600000; // 10 minutes
                       }),
        deviceTrackers.end());

    // Check if trackerMac is already in deviceTrackers
    for (auto &tracker : deviceTrackers)
    {
        if (strcmp(tracker.mac, trackerMac) == 0)
        {
            tracker.lastSeenTimestamp = currentTime;
            tracker.id = deviceId;
            return;
        }
    }

    // If not found, add new tracker
    deviceTracker newTracker;
    newTracker.id = deviceId;
    strncpy(newTracker.mac, trackerMac, sizeof(newTracker.mac));
    newTracker.mac[sizeof(newTracker.mac) - 1] = '\0';
    newTracker.lastSeenTimestamp = currentTime;
    deviceTrackers.push_back(newTracker);
}

void IdRoleManager::ensureUniqueId(int deviceId)
{
    // If deviceId is 0, generate a new ID
    int candidateId = deviceId;
    if (candidateId == 0)
    {
        candidateId = random(1, 1000001); // 1 to 1,000,000
    }

    bool unique = false;
    while (!unique)
    {
        unique = true;
        for (const auto &tracker : deviceTrackers)
        {
            if (tracker.id == candidateId && !WiFi.macAddress().equalsIgnoreCase(tracker.mac))
            {
                candidateId = random(1, 1000001);
                unique = false;
                break;
            }
        }
    }
    myId = candidateId;
}

// Returns a copy of the deviceTrackers list
std::vector<deviceTracker> IdRoleManager::getDeviceTrackers() const
{
    return deviceTrackers;
}

// Returns my own ID
int IdRoleManager::getMyId() const
{
    return myId;
}

// Returns true if this device has the lowest ID in deviceTrackers
bool IdRoleManager::isMaster() const
{
    for (const auto &tracker : deviceTrackers)
    {
        if (tracker.id < myId)
            return false;
    }
    return true;
}
// Callback function for test data reception
void IdRoleManager::onTestDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    struct struct_message
    {
        char id[20];
        uint8_t macAddress[6];
    } incomingMsg;

    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

    Serial.print("ID received from: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", mac[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.print(" - ID: ");
    Serial.println(incomingMsg.id);

    if (memcmp(mac, broadcastAddress2, 6) == 0 && !testReceivedFrom[0])
    {
        testReceivedFrom[0] = true;
        testReceivedCount++;
        strncpy(testReceivedIds[0], incomingMsg.id, sizeof(testReceivedIds[0]));
    }
    else if (memcmp(mac, broadcastAddress4, 6) == 0 && !testReceivedFrom[1])
    {
        testReceivedFrom[1] = true;
        testReceivedCount++;
        strncpy(testReceivedIds[1], incomingMsg.id, sizeof(testReceivedIds[1]));
    }

    if (testReceivedCount == 1)
    {
        Serial.println("All test IDs received!");

        int myIdInt = atoi(myId);
        int otherId = testReceivedFrom[0] ? atoi(testReceivedIds[0]) : atoi(testReceivedIds[1]);

        // Compare
        if (myIdInt < otherId)
        {
            Serial.println("You are the Master!");
        }
        else
        {
            Serial.println("You are NOT the Master!");
        }
    }
}