#include "Robot.h"

const int LEFT_MOTOR_ENABLE_PIN = 7;
const int LEFT_MOTOR_FORWARD_PIN = 5;
const int LEFT_MOTOR_BACKWARD_PIN = 6;
const int RIGHT_MOTOR_ENABLE_PIN = 2;
const int RIGHT_MOTOR_FORWARD_PIN = 3;
const int RIGHT_MOTOR_BACKWARD_PIN = 4;


const int MOVING_LED_PIN = 22;
const int BUTTON_PIN = 26;

Robot::Robot() 
    :   _buttonPressed(false),
        _motorController(
            LEFT_MOTOR_ENABLE_PIN,
            LEFT_MOTOR_FORWARD_PIN,
            LEFT_MOTOR_BACKWARD_PIN,
            RIGHT_MOTOR_ENABLE_PIN,
            RIGHT_MOTOR_FORWARD_PIN,
            RIGHT_MOTOR_BACKWARD_PIN
        )
{
    _motorController.stop();
    Serial.println("Robot::Robot");
    pinMode(MOVING_LED_PIN, OUTPUT);
    digitalWrite(MOVING_LED_PIN, LOW);

    pinMode(BUTTON_PIN, INPUT);
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
            buttonPressed = true;
        }
        _buttonPressed = true;
    } else {
        _buttonPressed = false;
    }
    return buttonPressed;
}

void Robot::turn(int degrees) {
    // TODO Auto-generated method stub

}

void Robot::move(int millimeters) {
    Serial.print("Robot::move(");
    Serial.print(millimeters);
    Serial.println(")");
    _motorController.setSpeed(96);
    digitalWrite(MOVING_LED_PIN, HIGH);
    _motorController.forward();
    delay(millimeters);
    _motorController.stop();
    digitalWrite(MOVING_LED_PIN, LOW);

    Serial.println("Robot::move done");
    
}
