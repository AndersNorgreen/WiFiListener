#ifndef STRUCT_MESSAGE_H
#define STRUCT_MESSAGE_H

#include <Arduino.h>

typedef struct struct_message {
  char id[20];
  uint8_t macAddress[6];
} struct_message;

typedef struct struct_sniff_message {
  uint8_t macAddress[6];
  char RSSI[4]; // RSSI value as a string
  char channel[3]; // Channel number as a string
  char timestamp[20]; // Timestamp as a string
  uint8_t device_macAddress[6]; // TODO remove this, it is not nescsary in the struct
} struct_sniff_message;

#endif // STRUCT_MESSAGE_H