#include <Arduino.h>
#include <idGenerator.h>

uint32_t IdGenerator::getRandomID() {
  return currentId;
}

void IdGenerator::initId() {

    randomSeed(esp_random());  // Better randomness using ESP32 hardware RNG
    currentId = random(1, 1000001);
    Serial.printf("Generated ID: %u\n", currentId);
    checkId();
}

void IdGenerator::checkId() {
    while (true) {
        Serial.println("Type 'n' to generate a new ID. Type 'a' to get the current ID.");

        while (!Serial.available()) {
            delay(10);
        }

        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.equalsIgnoreCase("n")) {
            Serial.println(input);
            initId();
            break;
        }
        else if (input.equalsIgnoreCase("a")) {
            Serial.println(input);
            Serial.printf("Current ID: %u\n", currentId);
            break;
        }
        else {
            Serial.println(input);
            Serial.println("Invalid input. Please type 'n' or 'a'.");
        }
    }
}