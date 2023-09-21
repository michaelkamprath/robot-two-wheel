#ifndef __ROBOT_H__
#define __ROBOT_H__
#include <L298NX2.h>
#include "SpeedModel.h"

void leftRotationCounterISR();
void rightRotationCounterISR();

class Robot {
private:
    bool _buttonPressed;
    L298NX2 _motorController;
    SpeedModel _speedModel;

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

    void turn(int degrees);
    void move(int millimeters);
};

#endif // __ROBOT_H__