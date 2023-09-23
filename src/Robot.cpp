#include "Robot.h"
#include "DataLogger.h"


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

const int CONTROLLER_SAMPLE_PERIOD = 100;       // milliseconds

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
    _speedModel.startSpeedControl(110);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());

     digitalWrite(MOVING_LED_PIN, HIGH);
    // turn right if degrees is positive, left if degrees is negative
    if (degrees > 0) {
        sprintf(DataLogger::commonBuffer(), "Robot::turn: turning right %d degrees with target disc hole count = %lu", degrees, target_disc_hole_count);
        INFO_LOG(DataLogger::commonBuffer());
        _motorController.forwardA();
        _motorController.backwardB();
    } else {
        sprintf(DataLogger::commonBuffer(), "Robot::turn: turning left %d degrees with target disc hole count = %lu", degrees, target_disc_hole_count);
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
        sprintf(
            DataLogger::commonBuffer(),
            "Robot::turn: Turning\n   left wheel counter : %lu\n   right wheel counter: %lu\n   left wheel power: %d\n   right wheel power: %d",
            _leftWheelCounter,
            _rightWheelCounter,
            _speedModel.getSpeedA(),
            _speedModel.getSpeedB()
        );
        DEBUG_LOG(DataLogger::commonBuffer());
    }
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);
    sprintf(
        DataLogger::commonBuffer(),
        "Robot::turn: complete\n   left wheel counter : %lu\n   right wheel counter: %lu",
        _leftWheelCounter,
        _rightWheelCounter
    );
    DEBUG_LOG(DataLogger::commonBuffer());
}   

void Robot::move(int millimeters) {
    sprintf(
        DataLogger::commonBuffer(),
        "Robot::move: moving %d millimeters",
        millimeters
    );
    INFO_LOG(DataLogger::commonBuffer());
    _speedModel.startSpeedControl(150);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());
    sprintf(
        DataLogger::commonBuffer(),
        "Robot::move: initial left power = %d, initial right power = %d",
        _speedModel.getSpeedA(),
        _speedModel.getSpeedB()
    );
    DEBUG_LOG(DataLogger::commonBuffer());
    
    // initialize counters
    _leftWheelCounter = 0;
    _rightWheelCounter = 0;

    int lastLeftWheelCounter = _leftWheelCounter;
    int lastRighWheelCounter = _rightWheelCounter;

    digitalWrite(MOVING_LED_PIN, HIGH);
    unsigned long startMillis = millis();
    _motorController.forward();
    unsigned long currentMillis = millis();
    unsigned long lastCheckinMillis = currentMillis;
    while (currentMillis -  startMillis < millimeters) {
        unsigned long deltaMillis = currentMillis - lastCheckinMillis;
        if (deltaMillis > CONTROLLER_SAMPLE_PERIOD) {
            int leftDelta = _leftWheelCounter - lastLeftWheelCounter;
            int rightDelta = _rightWheelCounter - lastRighWheelCounter;
            lastCheckinMillis = currentMillis;
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
                _motorController.forward();
            }
        }
        currentMillis = millis();
    }
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);
    int final_speed_left = _speedModel.getSpeedA();
    int final_speed_right = _speedModel.getSpeedB();
    sprintf(
        DataLogger::commonBuffer(),
        "Robot::move: complete, left wheel counter : %lu, right wheel counter: %lu, left wheel power: %d, right wheel power: %d",
        _leftWheelCounter,
        _rightWheelCounter,
        final_speed_left,
        final_speed_right
    );
    DEBUG_LOG(DataLogger::commonBuffer());
}
