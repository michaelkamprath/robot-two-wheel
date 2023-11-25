#ifndef __SPEEDMODEL_H__
#define __SPEEDMODEL_H__
#include <Arduino.h>
#include <PIDController.h>

class SpeedModel {
private:
    uint32_t _wheelDiscHoles;

    double _averageSpeed;
    double _lrRatio;
    int _speedA;
    int _speedB;

public:
    SpeedModel(
        uint32_t wheelDiscHoles = 20
    );
    virtual ~SpeedModel();

    void reset();

    void setAverageSpeed(uint8_t speed);

    uint8_t getSpeedA() const;      // left wheel
    uint8_t getSpeedB() const;      // right wheel

};

#endif // __SPEEDMODEL_H__