#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

struct WifiConfig {
    WifiConfig() {}; 
    WifiConfig& operator=(const WifiConfig& other) {
      if (this != &other) {
        SSID = other.SSID;
        Password = other.Password;
        IpAddress = other.IpAddress;
        Gateway = other.Gateway;
        Subnet = other.Subnet;
      }

      return *this;
    }

    String getSSID() const { return SSID; }
    String getPassword() const { return Password; }
    IPAddress getIpAddress() const { return IpAddress; }
    IPAddress getGateway() const { return Gateway; }
    IPAddress getSubnet() const { return Subnet; }
    String getIpAddressString() const { return IpAddress.toString(); }
    String getGatewayString() const { return Gateway.toString(); }
    String getSubnetString() const { return Subnet.toString(); }
    unsigned long getCurrentMillis() const { return CurrentMillis; }
    unsigned long getPreviousMillis() const { return PreviousMillis; }
    long getInterval() const { return Interval; }

    void setSSID(const String& newSSID) { SSID = newSSID; }
    void setPassword(const String& newPassword) { Password = newPassword; }
    void setIpAddress(const String& newIpAddress) { IpAddress.fromString(newIpAddress); }
    void setGateway(const String& newGateway) { Gateway.fromString(newGateway); }
    void setSubnet(const String& newSubnet) { Subnet.fromString(newSubnet); }
    void setIpAddress(const IPAddress& newIpAddress) { IpAddress = newIpAddress; }
    void setGateway(const IPAddress& newGateway) { Gateway = newGateway; }
    void setSubnet(const IPAddress& newSubnet) { Subnet = newSubnet; }
    void setCurrentMillis(){ CurrentMillis = millis(); }
    void setPreviousMillis(const unsigned long& newMillis) { PreviousMillis = newMillis; }
    void setInterval(const long& newInterval) { Interval = newInterval; }

  private:
    String SSID;
    String Password;
    IPAddress IpAddress;
    IPAddress Gateway;
    IPAddress Subnet;

    unsigned long PreviousMillis = 0;
    unsigned long CurrentMillis;
    long Interval = 10000;
};

#endif