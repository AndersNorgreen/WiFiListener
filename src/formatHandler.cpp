#include <Arduino.h>

String formatMac(String raw) {
    String formatted = "";
    for (int i = 0; i < raw.length(); i += 2) {
        if (i > 0) formatted += ":";
        formatted += raw.substring(i, i + 2);
    }
    return formatted;
}

String parseSSID(const uint8_t* payload, int len, int frameSubType) {
    int ssidOffset = 0;
    if (frameSubType == 8 || frameSubType == 5) { // Beacon or Probe Response
        ssidOffset = 36; // 24(header) + 12(fixed)
    } else if (frameSubType == 4) { // Probe Request
        ssidOffset = 24; // 24(header)
    } else {
        return "";
    }
    if (len <= ssidOffset + 2) return "";
    uint8_t tag = payload[ssidOffset];
    uint8_t tagLen = payload[ssidOffset + 1];
    if (tag != 0 || tagLen == 0 || tagLen > 32) return "";
    String ssid = "";
    for (int i = 0; i < tagLen; i++) {
        ssid += (char)payload[ssidOffset + 2 + i];
    }
    return ssid;
}
