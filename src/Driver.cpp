#include <Arduino.h>
#include "Driver.h"
#include "DataLogger.h"


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
        // _robot.turn(45);
        // delay(200);
        _robot.move(850);
        _isDriving = false;
        INFO_LOG(F("Driver::loop: driving done"));
    }
    else if (_robot.buttonPressed()) {
        INFO_LOG(F("Driver::loop: button pressed"));
        _isDriving = true;
    }
}

