#ifndef __HEADING_CALCULATOR_H__
#define __HEADING_CALCULATOR_H__
#include <MPU6050.h>


class HeadingCalculator
{
private:
    MPU6050 _mpu;
    float _heading;
    unsigned long _lastUpdate;

public:
    HeadingCalculator();
    ~HeadingCalculator();
    void reset(int init_heading = 0);
    float update();
    float getHeading()          { return this->update(); }

};

#endif // __HEADING_CALCULATOR_H__