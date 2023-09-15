#ifndef __ROBOT_H__
#define __ROBOT_H__
#include <L298NX2.h>


class Robot {
private:
    bool _buttonPressed;
    L298NX2 _motorController;

public:
    Robot();
    virtual ~Robot();
    void loop();

    bool buttonPressed();

    void turn(int degrees);
    void move(int millimeters);
};

#endif // __ROBOT_H__