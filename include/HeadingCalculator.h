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

    /// @brief Resets the heading to the specified value (default is 0 degrees)
    void reset(int init_heading = 0);

    /// @brief Updates the heading based on the current gyro reading. This function should be called
    /// at regular intervals to keep the heading up to date.
    /// @return The heding in degrees. Positive is counter-clockwise.
    float update();

    /// @brief Returns the current heading in degrees.
    /// @return The heading in degrees. will be between 180 and -180.
    float getHeading()          { return this->update(); }

};

#endif // __HEADING_CALCULATOR_H__