#include "SpeedModel.h"

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
        _speedA(0),
        _speedB(0)
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
                    Serial.print("eedModel::setAverageSpeed: Interpolating between ");
                    Serial.print(lrRatioPowerLevel[i-1]);
                    Serial.print(" and ");
                    Serial.print(lrRatioPowerLevel[i]);
                    Serial.print(" for power level ");
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

    Serial.print("SpeedModel::setAverageSpeed: Setting average speed to ");
    Serial.print(_averageSpeed);
    Serial.print(" and left/right ratio to ");
    Serial.print(leftRightRatio);
    Serial.print(", speed requested = ");
    Serial.println(speed);
}

void SpeedModel::reset() {
    _averageSpeed = 0.0;
    _speedA = 0;
    _speedB = 0;
}

uint8_t SpeedModel::getSpeedA() const {
    return _speedA;
}

uint8_t SpeedModel::getSpeedB() const {
    return _speedB;
}

void SpeedModel::updateSpeedsForStraightPath(
    unsigned long deltaMillis,  // Time since last update
    unsigned long totalMillis,  // Total time since start
    uint32_t deltaA,            // Counter delta for left motor A
    uint32_t deltaB,            // Counter delta for left motor B
    uint32_t counterA,          // Counter value for motor A since start
    uint32_t counterB           // Counter value for motor B since start
) {
    // the goal of this method is to adjust the left and right speed such that the
    // robot moves in a straight line. The primary signal used for this is the delta
    // between the left and right counters. If the delta is positive, then the left
    // wheel is moving faster than the right wheel, and the robot is turning to the
    // right. If the delta is negative, then the right wheel is moving faster than
    // the left wheel, and the robot is turning to the left.
    //
    // individual motor speed will be adjust up or down such that the difference between
    // the left and right delta is minimized, and the average speed between them is mainted
    // at _averageSpeed.
    if ( deltaA == deltaB) {
        // the robot is moving in a straight line, so no adjustment is needed
        Serial.println("SpeedModel::updateSpeedForCounter: Robot is moving in a straight line");
        return;
    }
    if ( deltaA > deltaB ) {
        //we want to decrease th left motor speed and increase the right motor speed such 
        // that the average between them is maintained at _averageSpeed
        // the amount of change is proportional to the difference between the two deltas
        // and the average speed
        float delta = float(deltaA - deltaB) / float(_wheelDiscHoles);
        float adjustment = delta * _averageSpeed;
        _speedA -= adjustment;
        _speedB = 2.0*_averageSpeed - _speedA;

        Serial.print("SpeedModel::updateSpeedForCounter: Turning right with deltaA = ");
    } else if ( deltaA < deltaB ) {
        //we want to increase th left motor speed and decrease the right motor speed such 
        // that the average between them is maintained at _averageSpeed
        // the amount of change is proportional to the difference between the two deltas
        // and the average speed
        float delta = float(deltaB - deltaA) / float(_wheelDiscHoles);
        float adjustment = delta * _averageSpeed;
        _speedA += adjustment;
        _speedB = 2.0*_averageSpeed - _speedA;
       Serial.print("SpeedModel::updateSpeedForCounter: Turning left with deltaA = ");
    }
    Serial.print(deltaA);
    Serial.print(", deltaB = ");
    Serial.print(deltaB);
    Serial.print(", _speedA = ");
    Serial.print(_speedA);
    Serial.print(", _speedB = ");
    Serial.println(_speedB);

    // now add a turn based on the total wheel count since the start. This will cause the
    // turn with the goal that the total wheel count will be the same for both wheels

    if (counterA > counterB) {
        // the left wheel has moved more than the right wheel, so we want to turn right
        // to decrease the left wheel count and increase the right wheel count
        _speedA -= counterA - counterB;
        Serial.print("SpeedModel::updateSpeedForCounter: Detected general left drift with counterA = ");
    } else if (counterB > counterA) {
        // the right wheel has moved more than the left wheel, so we want to turn left
        // to decrease the right wheel count and increase the left wheel count
        _speedB -= counterB - counterA;
        Serial.print("SpeedModel::updateSpeedForCounter: Detected general right drift with counterA = ");
    }
    Serial.print(counterA);
    Serial.print(", counterB = ");
    Serial.print(counterB);
    Serial.print(", _speedA = ");
    Serial.print(_speedA);
    Serial.print(", _speedB = ");
    Serial.println(_speedB);
}