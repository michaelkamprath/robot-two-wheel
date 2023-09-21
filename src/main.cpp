#include <Arduino.h>
#include "Driver.h"

const int STATUS_LED_PIN = 13;

Driver* driver;

void setup() {
    Serial.begin(57600);
    Serial.println(F("Kamprath Robot starting up..."));
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    driver = new Driver();
}

void loop() {
    driver->loop();
}
