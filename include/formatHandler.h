#ifndef FORMATHANDLER_H
#define FORMATHANDLER_H

#include <Arduino.h>

String formatMac(String raw);
String parseSSID(const uint8_t* payload, int len, int frameSubType);

#endif // FORMATHANDLER_H
