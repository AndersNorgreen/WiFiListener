#include <Arduino.h>
#include <id/idGenerator.h>

uint32_t IdGenerator::getRandomID() {
  return currentId;
}

void IdGenerator::initId() {

    randomSeed(esp_random());  // Better randomness using ESP32 hardware RNG
    currentId = random(1, 1000001);
    //currentId = random(1, 3);
    Serial.printf("Generated ID: %u\n", currentId);
}