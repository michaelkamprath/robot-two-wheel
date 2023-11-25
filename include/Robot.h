#ifndef __ROBOT_H__
#define __ROBOT_H__
#include <L298NX2.h>
#include "SpeedModel.h"
#include "Point.h"
#include "HeadingCalculator.h"

void leftRotationCounterISR();
void rightRotationCounterISR();

class Robot {
private:
    bool _buttonPressed;
    L298NX2 _motorController;
    SpeedModel _speedModel;
    HeadingCalculator _headingCalculator;

    uint32_t _leftWheelCounter;
    uint32_t _rightWheelCounter;



protected:
    friend void leftRotationCounterISR();
    friend void rightRotationCounterISR();

    void handleLeftWheelCounterISR();
    void handleRightWheelCounterISR();
public:
    static Robot* instance;

    Robot();
    virtual ~Robot();
    void loop();

    bool buttonPressed();

    // turn the robot by the specified number of degrees. A positive number turns the robot counter clockise (left), and a negative
    // number turns the robot clockwise (right) (all per the right hand rule).
    // returns the number of degrees the robot actually turned. This is useful for keeping track of the robot's orientation.
    int turn(int degrees);

    // move the robot forward or backward by the specified number of millimeters. A positive number moves the robot
    // forward, and a negative number moves the robot backward.
    // Returns the point the robot moved to relative to it's starting point, with axis y being the forward motion and axis X being any
    // horizontal deviation. This is useful for keeping track of the robot's position. A perfect forward motion would result in a point
    // with a y value equal to the number of millimeters moved, and an x value of 0.
    Point move(int millimeters);

    // The minimum size a turn needs to be for the robot to actually turn. This is to prevent the robot from turning
    // for very small turns, which would be a waste of time and probably inaccurate.
    // Expressed in degrees.
    int min_turn_angle() const;

    // The minimum distance the robot needs to move for the robot to actually move. This is to prevent the robot from
    // moving for very small distances, which would be a waste of time and probably inaccurate.
    int min_move_distance() const;
};

#endif // __ROBOT_H__