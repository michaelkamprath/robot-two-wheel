#include <Arduino.h>
#include "Robot.h"

const int STATUS_LED_PIN = 13;

Robot* robot;

void setup() {
    Serial.begin(57600);
    Serial.println("Kamprath Robot starting up...");
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    robot = new Robot();

    robot->move(2000);
}

void loop() {
    robot->loop();
}
