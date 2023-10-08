#include "SpeedModel.h"
#include "DataLogger.h"

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

const double lrRatioValue[POWER_RATIO_COUNT] = {
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

void SpeedModel::startSpeedControl(uint8_t speed) {
    double leftRightRatio = 1.0;
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
                    sprintf(
                        DataLogger::commonBuffer(),
                        "SpeedModel::startSpeedControl: Interpolating between %d and %d for power level %hu",
                        lrRatioPowerLevel[i-1],
                        lrRatioPowerLevel[i],
                        speed
                    );
                    DEBUG_LOG(DataLogger::commonBuffer());
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
    _averageSpeed = (double(_speedA) + double(_speedB))/ 2.0;
    _lrRatio = leftRightRatio;

    sprintf(
        DataLogger::commonBuffer(),
        "SpeedModel::startSpeedControl: Setting average speed to %s and left/right ratio to %s, speed requested = %hu",
        String(_averageSpeed,5).c_str(),
        String(leftRightRatio,5).c_str(),
        speed
    );
    DEBUG_LOG(DataLogger::commonBuffer());

    _pidController.begin();
    _pidController.setpoint(0.0);
    _pidController.tune(4, 3, 2);
    // _pidController.limit(-min(30, _averageSpeed), min(30, 255-_averageSpeed));
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

void SpeedModel::updateSpeedsForEqualRotation(
    uint32_t deltaA,            // Counter delta for left motor A
    uint32_t deltaB,            // Counter delta for left motor B
    uint32_t counterA,          // Counter value for motor A since start
    uint32_t counterB           // Counter value for motor B since start
) {
    int error = int(deltaA) - int(deltaB);
    _adjustment = _pidController.compute(error);
}