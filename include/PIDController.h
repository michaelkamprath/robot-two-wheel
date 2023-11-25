#ifndef __PID_CONTROLLER_H__
#define __PID_CONTROLLER_H__
#include <Arduino.h>

class PIDController
{
private:
    double _kp;
    double _ki;
    double _kd;
    double _setPoint;
    double _min;
    double _max;
    double _integral;
    double _lastError;
    unsigned long _lastTime;
    bool _firstTime;

public:
    PIDController(double kp, double ki, double kd, double min, double max);
    virtual ~PIDController();
    void setSetPoint(double setPoint);
    double update(double measurement, unsigned long measurement_millis);
    void reset();

    double getCumulativeError() const            { return this->_integral; }
};

#endif // __PID_CONTROLLER_H__