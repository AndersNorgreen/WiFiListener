#ifndef STRUCT_MESSAGE_H
#define STRUCT_MESSAGE_H

#include <Arduino.h>

typedef struct struct_message {
  char id[20];
  uint8_t macAddress[6];
} struct_message;

#endif // STRUCT_MESSAGE_H
