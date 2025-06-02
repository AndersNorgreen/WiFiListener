#ifndef SNIFF_AND_SEND_SERVICE_H
#define SNIFF_AND_SEND_SERVICE_H

#include <Arduino.h>
#include "struct_message.h"

class SniffAndSendService {
public:
    void sniff(int channel, int millisecs);
    void sniffCycleChannels(int millisecs);
    void sendSniffMessages(uint8_t masterAddress[6]);
private:
    struct_sniff_message buildSniffMessage(const uint8_t *macAddress, const char *rssi, const char *channel, const char *timestamp);
};

#endif // SNIFF_AND_SEND_SERVICE_H