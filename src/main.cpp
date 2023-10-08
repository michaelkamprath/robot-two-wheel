#include <Arduino.h>
#include "DataLogger.h"
#include "Driver.h"
#ifndef UNIT_TEST

#ifndef AUTO_VERSION
#define AUTO_VERSION "unknown"
#endif

const int STATUS_LED_PIN = 13;

Driver* driver;

void setup() {
    Serial.begin(57600);
    DataLogger::init();

    sprintf(DataLogger::commonBuffer(), "Kamprath Robot starting up with fimware version %s", AUTO_VERSION);
    INFO_LOG(DataLogger::commonBuffer());
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    driver = new Driver();
}

void loop() {
    driver->loop();
}

#endif // ndef UNIT_TEST