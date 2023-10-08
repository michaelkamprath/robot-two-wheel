#ifndef __SPEEDMODEL_H__
#define __SPEEDMODEL_H__
#include <Arduino.h>
#include <PIDController.h>

class SpeedModel {
private:
    uint32_t _wheelDiscHoles;

    double _averageSpeed;
    double _lrRatio;
    double _adjustment;
    int _speedA;
    int _speedB;

    PIDController _pidController;

public:
    SpeedModel(
        uint32_t wheelDiscHoles = 20
    );
    virtual ~SpeedModel();

    void reset();

    void startSpeedControl(uint8_t targetSpeed);
    void updateSpeedsForEqualRotation(
        uint32_t deltaA,            // Counter delta for motor A
        uint32_t deltaB,            // Counter delta for motor B
        uint32_t counterA,          // Counter value for motor A since start
        uint32_t counterB           // Counter value for motor B since start
    );

    uint8_t getSpeedA() const;      // left wheel
    uint8_t getSpeedB() const;      // right wheel

    double getCurrentAdjustment() const     { return _adjustment; }
    double getCumulativeError() const       { return _pidController.getErrorSum(); } 

};

#endif // __SPEEDMODEL_H__