#include "Robot.h"

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
        Serial.println("Robot::Robot: instance already exists");
    }

    _motorController.stop();
    Serial.println("Robot::Robot");
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
            Serial.println("Robot::buttonPressed: button pressed");
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
    char buffer[256];
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
    uint32_t target_disc_hole_count = DISC_HOLE_COUNT*fractionOfFullTurn -1;
    // initialize counters
    _leftWheelCounter = 0;
    _rightWheelCounter = 0;
    // power to 30%
    _speedModel.setAverageSpeed(102);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());

     digitalWrite(MOVING_LED_PIN, HIGH);
    // turn right if degrees is positive, left if degrees is negative
    if (degrees > 0) {
        sprintf(buffer, "Robot::turn: turning right %d degrees with target disc hole count = %lu", degrees, target_disc_hole_count);
        Serial.println(buffer);
        _motorController.forwardA();
        _motorController.backwardB();
    } else {
        sprintf(buffer, "Robot::turn: turning left %d degrees with target disc hole count = %lu", degrees, target_disc_hole_count);
        Serial.println(buffer);
        _motorController.backwardA();
        _motorController.forwardB();
    }

    while (_leftWheelCounter < target_disc_hole_count && _rightWheelCounter < target_disc_hole_count) {
        sprintf(
            buffer,
            "Robot::turn: Turning\n   left wheel counter : %lu\n   right wheel counter: %lu",
            _leftWheelCounter,
            _rightWheelCounter
        );
        Serial.println(buffer);
    }
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);
    sprintf(
        buffer,
        "Robot::turn: complete\n   left wheel counter : %lu\n   right wheel counter: %lu",
        _leftWheelCounter,
        _rightWheelCounter
    );
    Serial.println(buffer);
}   

void Robot::move(int millimeters) {
    char buffer[256];
    Serial.print(F("Robot::move("));
    Serial.print(millimeters);
    Serial.println(F(")"));
    _speedModel.setAverageSpeed(120);
    _motorController.setSpeedA(_speedModel.getSpeedA());
    _motorController.setSpeedB(_speedModel.getSpeedB());
    Serial.print(F("Robot::move: original left power = "));
    Serial.print(_speedModel.getSpeedA());
    Serial.print(F(", original right power = "));
    Serial.println(_speedModel.getSpeedB());
    
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
        if (deltaMillis > 100) {
            int leftDelta = _leftWheelCounter - lastLeftWheelCounter;
            int rightDelta = _rightWheelCounter - lastRighWheelCounter;
            lastCheckinMillis = currentMillis;
            lastLeftWheelCounter = _leftWheelCounter;
            lastRighWheelCounter = _rightWheelCounter;


            _speedModel.updateSpeedsForStraightPath(
                deltaMillis,
                currentMillis - startMillis,
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
        buffer,
        "Robot::move: complete\n   left wheel counter : %lu\n   right wheel counter: %lu\n   left wheel power: %d\n   right wheel power: %d",
        _leftWheelCounter,
        _rightWheelCounter,
        final_speed_left,
        final_speed_right
    );
    Serial.println(buffer);
}
