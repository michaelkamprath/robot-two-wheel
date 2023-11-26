#include "SpeedModel.h"
#include "DataLogger.h"

// the LR Power Ratio table is used to adjust the left/right power ratio based on the
// requested power level. The table was produced by setting both motors to the same
// power level and measuring the left/right ratio. The table is used to interpolate
// between the measured values.
#define POWER_RATIO_COUNT 12
const int lrRatioPowerLevel[POWER_RATIO_COUNT] = {
    70,
    80,
    90,
    100,
    110,
    120,
    140,
    160,
    180,
    200,
    225,
    255
};

const double lrRatioValue[POWER_RATIO_COUNT] = {
    1.00467,
    0.98837,
    1.00244,
    0.99218,
    0.98535,
    1.00406,
    1.00608,
    0.99909,
    0.97128,
    0.9729,
    0.95067,
    0.87813
};

const uint8_t MIN_POWER_SETTING = 70;

SpeedModel::SpeedModel(
    uint32_t wheelDiscHoles
)   :   _wheelDiscHoles(wheelDiscHoles),
        _averageSpeed(0.0),
        _lrRatio(1.0),
        _speedA(0),
        _speedB(0)
{
    this->reset();
}

SpeedModel::~SpeedModel() {

}

void SpeedModel::setAverageSpeed(uint8_t speed) {
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
                    sprintf_P(
                        DataLogger::commonBuffer(),
                        PSTR("SpeedModel::setAverageSpeed: Interpolating between %d and %d for power level %hu"),
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

    sprintf_P(
        DataLogger::commonBuffer(),
        PSTR("SpeedModel::setAverageSpeed: Setting average speed to %s and left/right ratio to %s, speed requested = %hu"),
        String(_averageSpeed,5).c_str(),
        String(leftRightRatio,5).c_str(),
        speed
    );
    DEBUG_LOG(DataLogger::commonBuffer());
}

void SpeedModel::reset() {
    _averageSpeed = 0.0;
    _lrRatio = 1.0;
    _speedA = 0;
    _speedB = 0;
}

uint8_t SpeedModel::getSpeedA() const {
    return max(min(_speedA, 255),MIN_POWER_SETTING);
}

uint8_t SpeedModel::getSpeedB() const {
    return max(min(_speedB, 255),MIN_POWER_SETTING);
}
