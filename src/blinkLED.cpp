#include <esp32-hal-gpio.h>

void blinkLED(int pin,  unsigned long duration) {
    pinMode(pin, OUTPUT);
    unsigned long current = millis();
    unsigned long waitTime = current + duration;
    while(current < waitTime)
        {
            digitalWrite(pin, HIGH);
            delay(100);
            digitalWrite(pin, LOW);
            delay(100);
            current = millis();
        }
}