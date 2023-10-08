#ifdef UNIT_TEST
#include <Arduino.h>
#include <unity.h>
#include "test_DataTable.h"

void runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_DataTable);
    RUN_TEST(test_DataTable_extend);
    RUN_TEST(test_DataTable_custom_formatter);
    UNITY_END();
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    runUnityTests();
}

void loop() {
}

#endif