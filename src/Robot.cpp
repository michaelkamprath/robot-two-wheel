#include "Robot.h"
#include "DataLogger.h"
#include "DataTable.h"
#include "PIDController.h"

const int LEFT_MOTOR_ENABLE_PIN = 7;
const int LEFT_MOTOR_FORWARD_PIN = 4;
const int LEFT_MOTOR_BACKWARD_PIN = 5;
const int RIGHT_MOTOR_ENABLE_PIN = 6;
const int RIGHT_MOTOR_FORWARD_PIN = 2;
const int RIGHT_MOTOR_BACKWARD_PIN = 3;

const int LEFT_ROTATION_COUNTER_PIN = 18;
const int RIGHT_ROTATION_COUNTER_PIN = 19;

const int MOVING_LED_PIN = 22;
const int BUTTON_PIN = 26;

const int DISC_HOLE_COUNT = 20;

const int CONTROLLER_SAMPLE_PERIOD = 80;       // milliseconds

const double WHEEL_CIRCUMFERENCE = 214; // millimeters
const double WHEEL_BASE = 132.5;          // millimeters

const uint8_t TARGET_SPEED = 140;       // 0-255
const uint8_t MIN_SPEED = 80;          // 0-255

const float HEADING_PID_CONTROLLER_KP = 20.0;
const float HEADING_PID_CONTROLLER_KI = 0.0;
const float HEADING_PID_CONTROLLER_KD = 0.0;

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
        _headingCalculator(),
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

int Robot::min_turn_angle() const {
    return ceil(180.0*(1.0/DISC_HOLE_COUNT));
}

int Robot::min_move_distance() const {
    return ceil(WHEEL_CIRCUMFERENCE/DISC_HOLE_COUNT);
}
void Robot::loop() {
    // unset button press if button is not pressed
    if (digitalRead(BUTTON_PIN) == HIGH) {
        _buttonPressed = false;
    }

    _headingCalculator.update();
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

int Robot::turn(int degrees) {
const int NUM_DATA_COLUMNS = 7;
    String column_headers[] {
        "timestamp",                // 0
        "left wheel counter",       // 1
        "right wheel counter",      // 2
        "heading",                  // 3
        "target heading",           // 4
        "heading error"             // 5
        "power"                     // 6
    };


    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::turn: turning %d degrees"),
        degrees
    );
    DEBUG_LOG(DataLogger::commonBuffer());

    if (abs(degrees) < min_turn_angle()) {
        sprintf_P(
            DataLogger::commonBuffer(),
            PSTR("Robot::turn: turning angle (%d) is less than minimum turn angle magnitude (%d)"),
            degrees,
            min_turn_angle()
        );
        DEBUG_LOG(DataLogger::commonBuffer());
        return 0;
    }
    DataTable<double> turn_data(NUM_DATA_COLUMNS, column_headers, 35);

    // use the heading calculator to keep track of the heading
    _headingCalculator.reset();

    uint8_t start_power = 150;
    uint8_t stop_power = 75;
    uint8_t current_power = start_power;
    double heading_error = degrees;

    unsigned long currentMillis = millis();
    unsigned long lastCheckinMillis = currentMillis;

    _motorController.setSpeed(start_power);
    if (degrees > 0) {
        _motorController.backwardA();
        _motorController.forwardB();
    } else {
        _motorController.forwardA();
        _motorController.backwardB();
    }

    turn_data.append_row(
        NUM_DATA_COLUMNS,
        double(currentMillis),
        double(_leftWheelCounter),
        double(_rightWheelCounter),
        _headingCalculator.getHeading(),
        double(degrees),
        heading_error,
        double(current_power)
    );
    while ((heading_error = fabs(degrees - _headingCalculator.getHeading())) > this->min_turn_angle()) {
        this->loop();
        currentMillis = millis();
        unsigned long deltaMillis = currentMillis - lastCheckinMillis;
        if (deltaMillis > CONTROLLER_SAMPLE_PERIOD) {
            lastCheckinMillis = currentMillis;
            sprintf_P(
                DataLogger::commonBuffer(),
                PSTR("Robot::turn: heading error = %f"),
                heading_error
            );
            DEBUG_LOG(DataLogger::commonBuffer());

            // calculate new power setting
            current_power = stop_power + min(heading_error/fabs(degrees), 1.0)*(start_power - stop_power);
            _motorController.stop();
            _motorController.setSpeed(start_power);
            if (degrees > 0) {
                _motorController.backwardA();
                _motorController.forwardB();
            } else {
                _motorController.forwardA();
                _motorController.backwardB();
            }
            turn_data.append_row(
                NUM_DATA_COLUMNS,
                double(currentMillis),
                double(_leftWheelCounter),
                double(_rightWheelCounter),
                _headingCalculator.getHeading(),
                double(degrees),
                heading_error,
                double(current_power)
            );
        }

    }

    _motorController.stop();
    _motorController.setSpeed(0);
    turn_data.append_row(
        NUM_DATA_COLUMNS,
        double(currentMillis),
        double(_leftWheelCounter),
        double(_rightWheelCounter),
        _headingCalculator.getHeading(),
        double(degrees),
        heading_error,
        double(current_power)
    );

    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::turn: complete, target degress: %d, heading: %s"),
        degrees,
        String(_headingCalculator.getHeading(),2).c_str()
    );
    DEBUG_LOG(DataLogger::commonBuffer());

    DEBUG_LOG(F("Robot::turn: the turn data:"));
    DataLogger::getInstance()->log_data_table(
        turn_data,
        [](double value, int col_num) -> String {
            switch(col_num) {
                case 0:
                case 1:
                case 2:
                case 4:
                case 6:
                default:
                    return String(value, 0);
                    break;
                case 3:
                case 5:
                    return String(value, 2);
                    break;
            }
        }
    );

    return _headingCalculator.getHeading();
}

Point Robot::move(int millimeters) {
    const int NUM_DATA_COLUMNS = 16;
    String column_headers[] {
        "timestamp",                        // 0
        "left wheel counter",               // 1
        "right wheel counter",              // 2
        "left wheel counter delta",         // 3
        "right wheel counter delta",        // 4
        "left wheel power",                 // 5
        "right wheel power",                // 6
        "forward distance increment",       // 7
        "forward distance total",           // 8
        "wheel turning angle",              // 9
        "wheel turning radius",             // 10
        "current wheel bearing",            // 11
        "current gyro heading",             // 12
        "target wheel tick count",          // 13
        "cumulative stearing error",        // 14
        "control signal"                    // 15
    };
    DataTable<double> move_data(NUM_DATA_COLUMNS, column_headers, 35);

    // first calculate wheel rotation count for the distance.
    uint32_t target_wheel_tick_count = (abs(millimeters) / WHEEL_CIRCUMFERENCE) * DISC_HOLE_COUNT + 1;
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::move: moving %d millimeters with target wheel tick count = %lu"),
        millimeters,
        target_wheel_tick_count
    );
    INFO_LOG(DataLogger::commonBuffer());

    // initialize speed model
    _speedModel.setAverageSpeed(TARGET_SPEED);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());
    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("Robot::move: initial left power = %d, initial right power = %d"),
        _speedModel.getSpeedA(),
        _speedModel.getSpeedB()
    );
    DEBUG_LOG(DataLogger::commonBuffer());

    // set up the controller
    PIDController controller(
        HEADING_PID_CONTROLLER_KP,
        HEADING_PID_CONTROLLER_KI,
        HEADING_PID_CONTROLLER_KD,
        -30,
        30
    );
    controller.setSetPoint(0.0); // keep heading straight
    DEBUG_LOG(F("Robot::move: controller initialized"));

    // initialize counters
    _leftWheelCounter = 0;
    _rightWheelCounter = 0;

    uint32_t lastLeftWheelCounter = _leftWheelCounter;
    uint32_t lastRighWheelCounter = _rightWheelCounter;

    double forward_distance = 0.0;
    double forward_distance_increment = 0.0;
    double horizontal_displacement = 0.0;
    double turning_angle = 0.0;
    double turning_radius = 0.0;
    double wheel_bearing = 0.0;
    bool slowDownInitiated = false;
    _headingCalculator.reset();

    digitalWrite(MOVING_LED_PIN, HIGH);
    DEBUG_LOG(F("Robot::move: starting motors"));
    _motorController.forward();
    unsigned long currentMillis = millis();
    unsigned long lastCheckinMillis = currentMillis;
    while ( (_leftWheelCounter < target_wheel_tick_count) || (_rightWheelCounter < target_wheel_tick_count)) {
        this->loop();
        currentMillis = millis();
        unsigned long deltaMillis = currentMillis - lastCheckinMillis;
        if (deltaMillis > CONTROLLER_SAMPLE_PERIOD) {
            lastCheckinMillis = currentMillis;
            uint32_t curLeftWheelCounter = _leftWheelCounter;
            uint32_t curRightWheelCounter = _rightWheelCounter;
            uint32_t leftDelta = curLeftWheelCounter - lastLeftWheelCounter;
            uint32_t rightDelta = curRightWheelCounter - lastRighWheelCounter;
            lastLeftWheelCounter = curLeftWheelCounter;
            lastRighWheelCounter = curRightWheelCounter;

            if (rightDelta > leftDelta) {
                turning_angle = CALC_TURNING_ANGLE(rightDelta, leftDelta);
                turning_radius = CALC_TURNING_RADIUS(rightDelta, leftDelta);
                forward_distance_increment = (turning_radius+WHEEL_BASE/2)*sin(turning_angle);
                horizontal_displacement += CALC_HORIZONTAL_DISTANCE(turning_radius, turning_angle);
            } else if (leftDelta > rightDelta) {
                turning_angle = CALC_TURNING_ANGLE(leftDelta, rightDelta);
                turning_radius = CALC_TURNING_RADIUS(leftDelta, rightDelta);
                forward_distance_increment = (turning_radius+WHEEL_BASE/2)*sin(turning_angle);
                turning_angle = -turning_angle;
                horizontal_displacement += CALC_HORIZONTAL_DISTANCE(turning_radius, turning_angle);
            } else {
                turning_angle = 0.0;
                forward_distance_increment = rightDelta*WHEEL_CIRCUMFERENCE/(double)DISC_HOLE_COUNT;
            }

            turning_angle *= 180.0/PI;
            forward_distance += forward_distance_increment;

            wheel_bearing += turning_angle;

            double gyro_heading = _headingCalculator.getHeading();
            double control_signal = controller.update(
                gyro_heading,
                currentMillis
            );

            uint8_t power_adjustment = (uint8_t)abs(control_signal);

            // positive control signal means turn left, a negative control signal means turn right
            if (control_signal > 0.0) {
                _motorController.setSpeedA(_speedModel.getSpeedA() - power_adjustment);
                _motorController.setSpeedB(_speedModel.getSpeedB() + power_adjustment);
            } else {
                _motorController.setSpeedA(_speedModel.getSpeedA() - power_adjustment);
                _motorController.setSpeedB(_speedModel.getSpeedB() + power_adjustment);
            }
            // need to call forward() again to set the PWN values
            _motorController.forward();

            move_data.append_row(
                NUM_DATA_COLUMNS,
                double(currentMillis),
                double(curLeftWheelCounter),
                double(curRightWheelCounter),
                double(leftDelta),
                double(rightDelta),
                double(_motorController.getSpeedA()),
                double(_motorController.getSpeedB()),
                forward_distance_increment,
                forward_distance,
                turning_angle,
                turning_radius,
                wheel_bearing,
                gyro_heading,
                double(target_wheel_tick_count),
                controller.getCumulativeError(),
                control_signal
            );
        }
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
        double(_motorController.getSpeedA()),
        double(_motorController.getSpeedB()),
        double(0),
        forward_distance,
        double(0),
        double(0),
        wheel_bearing,
        _headingCalculator.getHeading(),
        double(target_wheel_tick_count),
        controller.getCumulativeError(),
        0.0
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
                case 13:
                    return String(value, 0);
                    break;
                default:
                    return String(value, 2);
                    break;
                case 14:
                case 15:
                    if (value == 0.0) {
                        return String("0");
                    } else {
                        return String(value, 8);
                    }
                    break;
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
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

    return Point(horizontal_displacement, forward_distance);
}
