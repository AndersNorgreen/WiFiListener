#ifndef IDROLEMANAGER_H
#define IDROLEMANAGER_H

#include <Arduino.h>
#include <stdint.h>

class IdRoleManager{
    public:
    void init();
    void manageRoles();
    void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

    // Test functions
    void testInit();
    void manageTestRoles();
    void onTestDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

    private:
    bool compareMacArrays(const uint8_t* arr, const String& macStr);
    void blankMac(uint8_t* macArr);
    void sendIdTo(const uint8_t* mac);

    int receivedCount = 0;
    bool receivedFrom[4] = {false, false, false, false};
    char receivedIds[4][20];

    // Test variables
    int testReceivedCount = 0;
    bool testReceivedFrom[2] = {false, false};
    char testReceivedIds[2][20];
};

#endif