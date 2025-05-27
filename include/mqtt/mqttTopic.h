#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

#include <Arduino.h>

struct Coordinates {
  size_t x;
  size_t y;
};

struct MqttTopic {
  MqttTopic() {};
  MqttTopic(String sender, Coordinates location) { Sender = sender, Location = location; }

  String getSender() { return Sender; }
  Coordinates getLocation() { return Location; }
  size_t getX() { return Location.x; }
  size_t getY() { return Location.y; }

  void setSender(String sender) { Sender = sender; }
  void setLocation(Coordinates location) { Location = location; }
  void setX(size_t x) { Location.x = x; }
  void setY(size_t y) { Location.y = y; }
  private:
    String Sender;
    Coordinates Location;
};

#endif