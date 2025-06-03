#ifndef IDROLEMANAGER_H
#define IDROLEMANAGER_H

#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include <cstring>

class IdRoleManager{
    public:
    void init();
    void manageRoles();
    void updateDeviceInfoTracker(const char* mac, const char* id);
    int checkAndCompareRoles(String& masterMacAddress);
    void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

    private:
    struct struct_message {
        char id[20];
        uint8_t macAddress[6];
    };

    void sendIdTo(const uint8_t* mac);
    bool hasIdCollision();
    void rerollAndBroadcastId();

    int receivedCount = 0;
    bool receivedFrom[4] = {false, false, false, false};
    char receivedIds[4][20];

};

#endif