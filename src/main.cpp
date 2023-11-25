#include <Arduino.h>
#include <Wire.h>
#include "DataLogger.h"
#include "Driver.h"
#ifndef UNIT_TEST

#ifndef AUTO_VERSION
#define AUTO_VERSION "unknown"
#endif


Driver* driver;

void setup() {
    Wire.begin();
    Serial.begin(250000);
    DataLogger::init();

    sprintf(DataLogger::commonBuffer(), "Kamprath Robot starting up with fimware version %s", AUTO_VERSION);
    INFO_LOG(DataLogger::commonBuffer());

    driver = new Driver();
}

void loop() {
    driver->loop();
}

#endif