#ifndef __SPEEDMODEL_H__
#define __SPEEDMODEL_H__
#include <Arduino.h>
#include <PIDController.h>

class SpeedModel {
private:
    uint32_t _wheelDiscHoles;

    float _averageSpeed;
    float _lrRatio;
    float _adjustment;
    int _speedA;
    int _speedB;

    PIDController _pidController;

public:
    SpeedModel(
        uint32_t wheelDiscHoles = 20
    );
    virtual ~SpeedModel();

    void setAverageSpeed(uint8_t speed);
    void reset();

    void updateSpeedsForStraightPath(
        unsigned long deltaMillis,  // Time since last update
        unsigned long totalMillis,  // Total time since start
        uint32_t deltaA,            // Counter delta for motor A
        uint32_t deltaB,            // Counter delta for motor B
        uint32_t counterA,          // Counter value for motor A since start
        uint32_t counterB           // Counter value for motor B since start
    );

    uint8_t getSpeedA() const;      // left wheel
    uint8_t getSpeedB() const;      // right wheel

};

#endif // __SPEEDMODEL_H__