#include "PIDController.h"

/// @brief Creates a new PIDController
/// @param kp The proportional gain
/// @param ki The integral gain
/// @param kd The derivative gain
/// @param min The minimum control signal value to be outputted
/// @param max The maximum control signal value to be outputted
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

/// @brief Sets the set point of the controller.
/// @param set_point The desired of the measured variable
void PIDController::setSetPoint(double set_point)
{
    this->_setPoint = set_point;
}

/// @brief Updates the controller with a new measurement, generating a new control signal.
/// @param measurement The new measurement of the controlled variable
/// @param measurement_millis The time at which the measurement was taken in milliseconds
/// @return The new control signal
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

/// @brief Resets the controller, clearing the integral and derivative terms.
void PIDController::reset()
{
    this->_integral = 0;
    this->_lastError = 0;
    this->_lastTime = 0;
    this->_firstTime = true;
}
