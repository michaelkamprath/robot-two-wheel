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

    /// @brief Sets the set point for the PID controller.
    /// @param setPoint The set point.
    void setSetPoint(double setPoint);

    /// @brief Updates the PID controller with the specified measurement (note, not the error in the measurement).
    /// @param measurement The measurement.
    /// @param measurement_millis The time the measurement was taken in milliseconds.
    double update(double measurement, unsigned long measurement_millis);

    /// @brief Resets the PID controller.
    void reset();

    double getCumulativeError() const            { return this->_integral; }
};

#endif // __PID_CONTROLLER_H__