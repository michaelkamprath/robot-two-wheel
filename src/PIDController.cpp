#include "PIDController.h"

PIDController::PIDController(double kp, double ki, double kd, double min, double max) :
    _kp(kp),
    _ki(ki),
    _kd(kd),
    _setPoint(0),
    _min(min),
    _max(max),
    _integral(0),
    _lastError(0),
    _lastTime(0),
    _firstTime(true)
{
}

PIDController::~PIDController()
{
}

void PIDController::setSetPoint(double setPoint)
{
    this->_setPoint = setPoint;
}

double PIDController::update(double measurement, unsigned long measurement_millis)
{
    if (this->_firstTime) {
        this->_lastTime = measurement_millis;
        this->_firstTime = false;
        return 0;
    }
    double error = this->_setPoint - measurement;
    double dt = (measurement_millis - this->_lastTime) / 1000.0;
    this->_lastTime = measurement_millis;

    this->_integral += error * dt;
    double derivative = (error - this->_lastError) / dt;
    this->_lastError = error;

    double signal = this->_kp * error + this->_ki * this->_integral + this->_kd * derivative;

    if (signal > this->_max) {
        signal = this->_max;
    } else if (signal < this->_min) {
        signal = this->_min;
    }

    return signal;
}

void PIDController::reset()
{
    this->_integral = 0;
    this->_lastError = 0;
    this->_lastTime = 0;
    this->_firstTime = true;
}
