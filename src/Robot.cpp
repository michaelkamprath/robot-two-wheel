#include "Robot.h"
#include "DataLogger.h"
#include "DataTable.h"

const int LEFT_MOTOR_ENABLE_PIN = 7;
const int LEFT_MOTOR_FORWARD_PIN = 5;
const int LEFT_MOTOR_BACKWARD_PIN = 6;
const int RIGHT_MOTOR_ENABLE_PIN = 2;
const int RIGHT_MOTOR_FORWARD_PIN = 3;
const int RIGHT_MOTOR_BACKWARD_PIN = 4;

const int LEFT_ROTATION_COUNTER_PIN = 20;
const int RIGHT_ROTATION_COUNTER_PIN = 21;

const int MOVING_LED_PIN = 22;
const int BUTTON_PIN = 26;

const int DISC_HOLE_COUNT = 20;

const int CONTROLLER_SAMPLE_PERIOD = 80;       // milliseconds

const double WHEEL_CIRCUMFERENCE = 214; // millimeters
const double WHEEL_BASE = 132.5;          // millimeters

const uint8_t TARGET_SPEED = 160;       // 0-255
const uint8_t MIN_SPEED = 100;          // 0-255

// Turning angle formula (in radians):
//
//   angle = WHEEL_CIRCUMFERENCE*((outside_wheel_count - inside_wheel_count)/DISC_HOLE_COUNT)/WHEEL_BASE
//
// The only variables are the outside_wheel_count and inside_wheel_count. We can calculate and turning angle
// factor based off the contstant values.
//
//   angle_factor = WHEEL_CIRCUMFERENCE/DISC_HOLE_COUNT/WHEEL_BASE
//
// the turning angle is then:
//
//   angle = angle_factor*(outside_wheel_count - inside_wheel_count)

const double TURNING_ANGLE_FACTOR = WHEEL_CIRCUMFERENCE/DISC_HOLE_COUNT/WHEEL_BASE;

#define CALC_TURNING_ANGLE(outside_wheel_count, inside_wheel_count) (TURNING_ANGLE_FACTOR*((outside_wheel_count) - (inside_wheel_count)))
#define CALC_TURNING_RADIUS(outside_wheel_count, inside_wheel_count) (WHEEL_BASE*inside_wheel_count/(outside_wheel_count - inside_wheel_count))
#define CALC_HORIZONTAL_DISTANCE(turning_radius, turning_angle) ((turning_radius+WHEEL_BASE/2)*(1.0 - cos(turning_angle)))
#define CALC_VERTICAL_DISTANCE(turning_radius, turning_angle, wheel_rotation_ticks) (sin(turning_angle) != 0.0 ? (turning_radius+WHEEL_BASE/2)*sin(turning_angle) : wheel_rotation_ticks*WHEEL_CIRCUMFERENCE/DISC_HOLE_COUNT)
//
// Interupt Service Routines
//
Robot* Robot::instance = nullptr;

void leftRotationCounterISR() {
    Robot::instance->handleLeftWheelCounterISR();
}

void rightRotationCounterISR() {
    Robot::instance->handleRightWheelCounterISR();
}


Robot::Robot() 
    :   _buttonPressed(false),
        _motorController(
            LEFT_MOTOR_ENABLE_PIN,
            LEFT_MOTOR_FORWARD_PIN,
            LEFT_MOTOR_BACKWARD_PIN,
            RIGHT_MOTOR_ENABLE_PIN,
            RIGHT_MOTOR_FORWARD_PIN,
            RIGHT_MOTOR_BACKWARD_PIN
        ),
        _speedModel(DISC_HOLE_COUNT),
        _leftWheelCounter(0),
        _rightWheelCounter(0)
{
    if (instance == nullptr) {
        instance = this;
    } else {
        ERROR_LOG("Robot::Robot: instance already exists");
    }

    _motorController.stop();
    pinMode(MOVING_LED_PIN, OUTPUT);
    digitalWrite(MOVING_LED_PIN, LOW);

    pinMode(BUTTON_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(LEFT_ROTATION_COUNTER_PIN), leftRotationCounterISR, RISING);
    attachInterrupt(digitalPinToInterrupt(RIGHT_ROTATION_COUNTER_PIN), rightRotationCounterISR, RISING);
}

Robot::~Robot() {
    // TODO Auto-generated destructor stub
}

void Robot::loop() {
    // unset button press if button is not pressed
    if (digitalRead(BUTTON_PIN) == HIGH) {
        _buttonPressed = false;
    }
}

// checks button state. Returns true if button is newly pressed
bool Robot::buttonPressed() {
    bool buttonPressed = false;
    // button is active low
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!_buttonPressed) {
            DEBUG_LOG("Robot button pressed");
            buttonPressed = true;
        }
        _buttonPressed = true;
    } else {
        _buttonPressed = false;
    }
    return buttonPressed;
}

void Robot::handleLeftWheelCounterISR() {
    _leftWheelCounter++;
}

void Robot::handleRightWheelCounterISR() {
    _rightWheelCounter++;
}

void Robot::turn(int degrees) {
    // a 180 degree turn would be one full turn of each wheel in opoosite directions.
    // first task would be to calculate what fraction of a full turn each wheel needs to make.
    // convert the angle to equavilent value on range of -180 degrees to 180 degrees
    if (degrees > 180) {
        degrees -= 360;
    } else if (degrees < -180) {
        degrees += 360;
    }
    sprintf_P(DataLogger::commonBuffer(), PSTR("Robot::turn: turning %d degrees"), degrees);
    INFO_LOG(DataLogger::commonBuffer());

    // convert the angle to a fraction of a full turn
    float fractionOfFullTurn = fabs(degrees) / 180.0;
    // convert the fraction of a full turn to a fraction of the number of holes in the disc
    uint32_t target_disc_hole_count = DISC_HOLE_COUNT*fractionOfFullTurn;
    // initialize counters
    _leftWheelCounter = 0;
    _rightWheelCounter = 0;
    int lastLeftWheelCounter = _leftWheelCounter;
    int lastRighWheelCounter = _rightWheelCounter;
    // power to 30%
    _speedModel.startSpeedControl(150);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());

     digitalWrite(MOVING_LED_PIN, HIGH);
    // turn right if degrees is positive, left if degrees is negative
    if (degrees > 0) {
        sprintf_P(DataLogger::commonBuffer(), PSTR("Robot::turn: turning right %d degrees with target disc hole count = %lu"), degrees, target_disc_hole_count);
        INFO_LOG(DataLogger::commonBuffer());
        _motorController.forwardA();
        _motorController.backwardB();
    } else {
        sprintf_P(DataLogger::commonBuffer(), PSTR("Robot::turn: turning left %d degrees with target disc hole count = %lu"), degrees, target_disc_hole_count);
        INFO_LOG(DataLogger::commonBuffer());
        _motorController.backwardA();
        _motorController.forwardB();
    }

    unsigned long currentMillis = millis();
    unsigned long lastCheckinMillis = currentMillis;
    while (_leftWheelCounter < target_disc_hole_count && _rightWheelCounter < target_disc_hole_count) {
        unsigned long deltaMillis = currentMillis - lastCheckinMillis;
        if (deltaMillis > CONTROLLER_SAMPLE_PERIOD/2) {
            int leftDelta = _leftWheelCounter - lastLeftWheelCounter;
            int rightDelta = _rightWheelCounter - lastRighWheelCounter;
            lastLeftWheelCounter = _leftWheelCounter;
            lastRighWheelCounter = _rightWheelCounter;
            _speedModel.updateSpeedsForEqualRotation(
                leftDelta,
                rightDelta,
                _leftWheelCounter,
                _rightWheelCounter
            );
            if ((_speedModel.getSpeedA() != _motorController.getSpeedA()) || (_speedModel.getSpeedB() != _motorController.getSpeedB())) {
                // must stop motors to change speed
                _motorController.stop();
                _motorController.setSpeedA(_speedModel.getSpeedA());
                _motorController.setSpeedB(_speedModel.getSpeedB());
                if (degrees > 0) {
                    _motorController.forwardA();
                    _motorController.backwardB();
                } else {
                    _motorController.backwardA();
                    _motorController.forwardB();
                }                   
            }
        }
        sprintf_P(
            DataLogger::commonBuffer(),
            PSTR("Robot::turn: Turning\n   left wheel counter : %lu\n   right wheel counter: %lu\n   left wheel power: %d\n   right wheel power: %d"),
            _leftWheelCounter,
            _rightWheelCounter,
            _speedModel.getSpeedA(),
            _speedModel.getSpeedB()
        );
        DEBUG_LOG(DataLogger::commonBuffer());
    }
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::turn: complete\n   left wheel counter : %lu\n   right wheel counter: %lu"),
        _leftWheelCounter,
        _rightWheelCounter
    );
    DEBUG_LOG(DataLogger::commonBuffer());
}   

void Robot::move(int millimeters) {
    const int NUM_DATA_COLUMNS = 15;
    String column_headers[] {
        "timestamp",
        "left wheel counter",
        "right wheel counter",
        "left wheel counter delta",
        "right wheel counter delta",
        "left wheel power",
        "right wheel power",
        "forward distance increment",
        "forward distance total", 
        "turning angle",
        "turning radius",
        "current bearing",
        "target wheel tick count",
        "cumulative stearing error",
        "current stearing adjustment"
    };
    DataTable<double> move_data(NUM_DATA_COLUMNS, column_headers, 50);
    
    // first calculate wheel rotation count for the distance.
    uint32_t target_wheel_tick_count = (abs(millimeters) / WHEEL_CIRCUMFERENCE) * DISC_HOLE_COUNT + 1;
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::move: moving %d millimeters with target wheel tick count = %lu"),
        millimeters,
        target_wheel_tick_count
    );
    INFO_LOG(DataLogger::commonBuffer());

    _speedModel.startSpeedControl(TARGET_SPEED);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::move: initial left power = %d, initial right power = %d"),
        _speedModel.getSpeedA(),
        _speedModel.getSpeedB()
    );
    DEBUG_LOG(DataLogger::commonBuffer());
    
    // initialize counters
    _leftWheelCounter = 0;
    _rightWheelCounter = 0;

    uint32_t lastLeftWheelCounter = _leftWheelCounter;
    uint32_t lastRighWheelCounter = _rightWheelCounter;

    double forward_distance = 0.0;
    double horizontal_displacement = 0.0;
    double turning_angle = 0.0;
    double turning_radius = 0.0;
    double cur_bearing = 0.0;
    bool slowDownInitiated = false;

    digitalWrite(MOVING_LED_PIN, HIGH);
    _motorController.forward();
    unsigned long currentMillis = millis();
    unsigned long lastCheckinMillis = currentMillis;
    while ( (_leftWheelCounter < target_wheel_tick_count) || (_rightWheelCounter < target_wheel_tick_count)) {
        unsigned long deltaMillis = currentMillis - lastCheckinMillis;
        if (deltaMillis > CONTROLLER_SAMPLE_PERIOD) {
            uint32_t curLeftWheelCounter = _leftWheelCounter;
            uint32_t curRightWheelCounter = _rightWheelCounter;
            int leftDelta = curLeftWheelCounter - lastLeftWheelCounter;
            int rightDelta = curRightWheelCounter - lastRighWheelCounter;
            lastCheckinMillis = currentMillis;
            lastLeftWheelCounter = curLeftWheelCounter;
            lastRighWheelCounter = curRightWheelCounter;


            _speedModel.updateSpeedsForEqualRotation(
                leftDelta,
                rightDelta,
                curLeftWheelCounter,
                curRightWheelCounter
            );



            // calculate the horizontal displacement
            double forward_distance_increment = 0.0;
            if (rightDelta > leftDelta) {
                // turning left
                turning_angle = CALC_TURNING_ANGLE(rightDelta, leftDelta);
                cur_bearing += turning_angle;
                turning_radius = CALC_TURNING_RADIUS(rightDelta, leftDelta);
                forward_distance_increment = CALC_VERTICAL_DISTANCE(turning_radius, turning_angle, rightDelta);
                horizontal_displacement -= CALC_HORIZONTAL_DISTANCE(turning_radius, turning_angle);
            }
            else if (leftDelta > rightDelta) {
                // turning right
                turning_angle = -CALC_TURNING_ANGLE(leftDelta, rightDelta);
                cur_bearing += turning_angle;
                turning_radius = CALC_TURNING_RADIUS(leftDelta, rightDelta);
                forward_distance_increment = CALC_VERTICAL_DISTANCE(turning_radius, -turning_angle, leftDelta);
                horizontal_displacement += -CALC_HORIZONTAL_DISTANCE(turning_radius, -turning_angle);
            }
            else {
                // going straight
                turning_angle = 0.0;
                turning_radius = 0.0;
                forward_distance_increment = WHEEL_CIRCUMFERENCE*leftDelta/DISC_HOLE_COUNT;
            }
            forward_distance += forward_distance_increment;

            // if we are less than 20% of the target distance, then we need to start slowing down
            if (!slowDownInitiated && (forward_distance >= 0.8*fabs(millimeters))) {
                _speedModel.setAverageSpeed(MIN_SPEED);
                slowDownInitiated = true;
            }

            if ((_speedModel.getSpeedA() != _motorController.getSpeedA()) || (_speedModel.getSpeedB() != _motorController.getSpeedB())) {
                // must stop motors to change speed
                _motorController.stop();
                _motorController.setSpeedA(_speedModel.getSpeedA());
                _motorController.setSpeedB(_speedModel.getSpeedB());
                _motorController.forward();
            }
            // update target wheel tick count
            double remaining_distance = fabs(millimeters) - forward_distance;
            if (remaining_distance > 0) {
                target_wheel_tick_count =
                        (curLeftWheelCounter + curRightWheelCounter)/2 
                        + (remaining_distance / WHEEL_CIRCUMFERENCE) * DISC_HOLE_COUNT
                        + 1;
            } else {
                target_wheel_tick_count = (curLeftWheelCounter + curRightWheelCounter)/2;
            }

            move_data.append_row(
                NUM_DATA_COLUMNS,
                double(currentMillis),
                double(curLeftWheelCounter),
                double(curRightWheelCounter),
                double(leftDelta),
                double(rightDelta),
                double(_speedModel.getSpeedA()),
                double(_speedModel.getSpeedB()),
                forward_distance_increment,
                forward_distance,
                turning_angle,
                turning_radius,
                cur_bearing,
                double(target_wheel_tick_count),
                _speedModel.getCumulativeError(),
                _speedModel.getCurrentAdjustment()
            );
        }
        currentMillis = millis();
    }
    _motorController.stop();
    // ensure that the robot has stopped moving by reversing for a short time
    _motorController.backward();
    delay(100);
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);

    // capture final state
    move_data.append_row(
        NUM_DATA_COLUMNS,
        double(currentMillis),
        double(_leftWheelCounter),
        double(_rightWheelCounter),
        double(0),
        double(0),
        double(_speedModel.getSpeedA()),
        double(_speedModel.getSpeedB()),
        double(0),
        forward_distance,
        double(0),
        double(0),
        cur_bearing,
        double(target_wheel_tick_count),
        _speedModel.getCumulativeError(),
        _speedModel.getCurrentAdjustment()
    );

    int final_speed_left = _speedModel.getSpeedA();
    int final_speed_right = _speedModel.getSpeedB();
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::move: complete, left wheel counter : %lu, right wheel counter: %lu, left wheel power: %d, right wheel power: %d"),
        _leftWheelCounter,
        _rightWheelCounter,
        final_speed_left,
        final_speed_right
    );
    DEBUG_LOG(DataLogger::commonBuffer());

    DEBUG_LOG(F("Robot::move: the movement data:"));
    DataLogger::getInstance()->log_data_table(
        move_data, 
        [](double value, int col_num) -> String {
            switch(col_num) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 11:
                    return String(value, 0);
                    break;
                default:
                    return String(value, 2);
                    break;
                case 8:
                case 10:
                case 13:
                    if (value == 0.0) {
                        return String("0");
                    } else {
                        return String(value, 8);
                    }
                    break;
                case 7:
                case 9:
                case 12:
                    if (value == 0.0) {
                        return String("0");
                    } else {
                        return String(value, 2);
                    }
                    break;
            }
        }
    );
}
