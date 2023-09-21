#include "SpeedModel.h"

// the LR Power Ratio table is used to adjust the left/right power ratio based on the
// requested power level. The table was produced by setting both motors to the same
// power level and measuring the left/right ratio. The table is used to interpolate
// between the measured values. 
#define POWER_RATIO_COUNT 9
const int lrRatioPowerLevel[POWER_RATIO_COUNT] = {
    60,
    75,
    90,
    105,
    125,
    175,
    200,
    225,
    255
};

const float lrRatioValue[POWER_RATIO_COUNT] = {
    0.7962962963,
    0.8511530398,
    0.8844765343,
    0.8977272727,
    0.9349112426,
    0.94921875,
    0.9498746867,
    0.9377990431,
    0.9471871412
};

SpeedModel::SpeedModel(
    uint32_t wheelDiscHoles
)   :   _wheelDiscHoles(wheelDiscHoles),
        _averageSpeed(0.0),
        _lrRatio(1.0),
        _adjustment(0.0),
        _speedA(0),
        _speedB(0),
        _pidController()
{
    this->reset();
}

SpeedModel::~SpeedModel() {

}

void SpeedModel::setAverageSpeed(uint8_t speed) {
    float leftRightRatio = 1.0;
    if (speed <= lrRatioPowerLevel[0]) {
        speed = lrRatioPowerLevel[0];
        leftRightRatio = lrRatioValue[0];
    } else {
        // first, find the first entry into the lrRatioPowerLevel array that is 
        // less than or equal to the power level. If the first entry is greater
        // than the power level, then set the left/right ratio to the first entry
        for (int i = 0; i < POWER_RATIO_COUNT; i++) {
            if (lrRatioPowerLevel[i] > speed) {
                if (i < POWER_RATIO_COUNT - 1) {
                    Serial.print(F("eedModel::setAverageSpeed: Interpolating between "));
                    Serial.print(lrRatioPowerLevel[i-1]);
                    Serial.print(F(" and "));
                    Serial.print(lrRatioPowerLevel[i]);
                    Serial.print(F(" for power level "));
                    Serial.println(speed);
                    // interpolate this value and the next value
                    leftRightRatio = lrRatioValue[i] 
                        + (speed - lrRatioPowerLevel[i]) * (lrRatioValue[i] - lrRatioValue[i-1])
                                                        / (lrRatioPowerLevel[i] - lrRatioPowerLevel[i-1]);
                } else {
                    leftRightRatio = lrRatioValue[i];
                }
                break;
            }
            if (i == POWER_RATIO_COUNT - 1) {
                speed = lrRatioPowerLevel[POWER_RATIO_COUNT - 1];
                leftRightRatio = lrRatioValue[POWER_RATIO_COUNT - 1];
            }
        }
    }

    _speedA = speed;
    _speedB = speed * leftRightRatio;
    _averageSpeed = (float(_speedA) + float(_speedB))/ 2.0;
    _lrRatio = leftRightRatio;

    Serial.print(F("SpeedModel::setAverageSpeed: Setting average speed to "));
    Serial.print(_averageSpeed);
    Serial.print(F(" and left/right ratio to "));
    Serial.print(leftRightRatio);
    Serial.print(F(", speed requested = "));
    Serial.println(speed);

    _pidController.begin();
    _pidController.setpoint(0.0);
    _pidController.tune(4, 2, 2);
    _pidController.limit(-20, 20);
}

void SpeedModel::reset() {
    _averageSpeed = 0.0;
    _lrRatio = 1.0;
    _adjustment = 0.0;
    _speedA = 0;
    _speedB = 0;
}

uint8_t SpeedModel::getSpeedA() const {
    return max(min(_speedA + _adjustment, 255),0);
}

uint8_t SpeedModel::getSpeedB() const {
    return max(min(_speedB - _adjustment, 255),0);
}

void SpeedModel::updateSpeedsForStraightPath(
    unsigned long deltaMillis,  // Time since last update
    unsigned long totalMillis,  // Total time since start
    uint32_t deltaA,            // Counter delta for left motor A
    uint32_t deltaB,            // Counter delta for left motor B
    uint32_t counterA,          // Counter value for motor A since start
    uint32_t counterB           // Counter value for motor B since start
) {
    int error = int(counterA) - int(counterB);
    _adjustment = _pidController.compute(error);

    Serial.print(F("SpeedModel::updateSpeedForCounter: error = "));
    Serial.print(error);
    Serial.print(F(", adjustment = "));
    Serial.print(_adjustment);
    Serial.print(F(", _speedA = "));
    Serial.print(this->getSpeedA());
    Serial.print(F(", _speedB = "));
    Serial.println(this->getSpeedB());

}