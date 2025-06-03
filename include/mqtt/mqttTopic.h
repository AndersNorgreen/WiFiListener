#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

#include <Arduino.h>

struct Coordinates {
  int x;
  int y;
};

struct MqttTopic {
  MqttTopic() {};
  MqttTopic(String sender, Coordinates location) { Sender = sender, Location = location; }

  String getSender() { return Sender; }
  Coordinates getLocation() { return Location; }
  int getX() { return Location.x; }
  int getY() { return Location.y; }

  void setSender(String sender) { Sender = sender; }
  void setLocation(Coordinates location) { Location = location; }
  void setX(int x) { Location.x = x; }
  void setY(int y) { Location.y = y; }
  private:
    String Sender;
    Coordinates Location;
};

#endif