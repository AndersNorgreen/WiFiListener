#ifndef ESPNOWHANDLER_H
#define ESPNOWHANDLER_H

#include "struct_message.h"
#include <arduino.h>

void setupEspNow();
void buildMessage();
void broadcastMessage();
void sendSniffMessage(const struct_sniff_message &sniffMsg, uint8_t masterAddress[6]);

#endif // ESPNOWHANDLER_H
