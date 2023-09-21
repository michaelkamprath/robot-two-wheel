#include <Arduino.h>
#include "Driver.h"


Driver::Driver()
    :   _isDriving(false),
        _robot()
{

}

Driver::~Driver() {
    // TODO Auto-generated destructor stub
}

void Driver::loop() {
    _robot.loop();
    if (_isDriving) {
        // _robot.turn(-90);
        _robot.move(4000);
        _isDriving = false;
        Serial.println("Driver::loop: driving done");
    }
    else if (_robot.buttonPressed()) {
        Serial.println("Driver::loop: button pressed");
        _isDriving = true;
    }
}

