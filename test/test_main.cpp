#include <Arduino.h>
#include <unity.h>
#include "test_DataTable.h"
#include "test_PointSequence.h"

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

int runUnityTests(void) {
    UNITY_BEGIN();

    // Data Table
    RUN_TEST(test_DataTable);
    RUN_TEST(test_DataTable_extend);
    RUN_TEST(test_DataTable_custom_formatter);

    // Point Sequence
    RUN_TEST(test_Point_math);
    RUN_TEST(test_PointSequence);
    return UNITY_END();
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(3000);

    runUnityTests();
}

void loop() {
}
