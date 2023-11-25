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
        PointSequence path;
        path.add(Point(0, 0));
        path.add(Point(0, 2000));
        trace_path(path);
        INFO_LOG(F("Driver::loop: driving done"));
        _isDriving = false;
    }
    else if (_robot.buttonPressed()) {
        INFO_LOG(F("Driver::loop: button pressed"));
        _isDriving = true;
    }
}

void Driver::trace_path(const PointSequence& path) {
    if (path.size() <= 1) {
        ERROR_LOG(F("Driver::trace_path: path size is too small"));
        return;
    }
    Point current_point = path[0];
    double current_bearing = 0;

    for (uint16_t i = 1; i < path.size(); i++) {
        Point next_point = path[i];

        double distance = current_point.distance(next_point);
        double bearing = current_point.absolute_bearing(next_point);
        double bearing_delta = bearing - current_bearing;

        sprintf_P(
            DataLogger::commonBuffer(),
            PSTR("Driver::trace_path: current_point=%s, next_point=%s, distance=%s, bearing=%s, bearing_delta=%s"),
            String(current_point).c_str(),
            String(next_point).c_str(),
            String(distance).c_str(),
            String(bearing).c_str(),
            String(bearing_delta).c_str()
        );
        INFO_LOG(DataLogger::commonBuffer());

        int turn_results = 0;
        if (abs(bearing_delta) >= _robot.min_turn_angle()) {
            turn_results = _robot.turn(bearing_delta);
            delay(200);
        }
        sprintf_P(
            DataLogger::commonBuffer(),
            PSTR("Driver::trace_path: completed turn, turn_results=%d"),
            turn_results
        );
        INFO_LOG(DataLogger::commonBuffer());

        Point move_results;
        if (distance >= _robot.min_move_distance()) {
            move_results = _robot.move(distance);
        }
        sprintf_P(
            DataLogger::commonBuffer(),
            PSTR("Driver::trace_path: completed forward move, move_results=%s"),
            String(move_results).c_str()
        );
        INFO_LOG(DataLogger::commonBuffer());
        delay(200);
        current_point = next_point;
        current_bearing = bearing;
    }
}