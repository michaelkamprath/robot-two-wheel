#include <Arduino.h>
#include <Wire.h>
#include "HeadingCalculator.h"
#include "DataLogger.h"


const unsigned long HEADING_CALCULATOR_UPDATE_INTERVAL_MILLIS = 50;
const double GYRO_LSB_PER_DPS = 131;
const MPU6050_IMU::GYRO_FS GYRO_FULL_SCALE = MPU6050_IMU::GYRO_FS::MPU6050_GYRO_FS_250;

// CALIBRATION
// ....................	XAccel			YAccel				ZAccel			XGyro			YGyro			ZGyro
// [-2505,-2504] --> [-4,8]	[385,386] --> [-2,14]	[1559,1560] --> [16371,16394]	[82,83] --> [0,5]	[31,31] --> [0,2]	[-49,-48] --> [-1,2]
//  .................... [-2505,-2504] --> [-1,8]	[385,386] --> [-4,14]	[1559,1560] --> [16368,16394]	[82,82] --> [0,1]	[31,31] --> [0,1]	[-49,-48] --> [-1,2]
// -------------- done --------------


HeadingCalculator::HeadingCalculator()
    :   _mpu(),
        _heading(0)
{
    _mpu.initialize();
    _mpu.setFullScaleGyroRange(GYRO_FULL_SCALE);

    // set the mpu6050 offsets. These were determined by running the calibration code in the
    // Arduino C++ library. The calibration code is here:
    //      https://github.com/ElectronicCats/mpu6050/blob/master/examples/IMU_Zero/IMU_Zero.ino
    //
    _mpu.setXAccelOffset(-2505);
    _mpu.setYAccelOffset(385);
    _mpu.setZAccelOffset(1559);
    _mpu.setXGyroOffset(82);
    _mpu.setYGyroOffset(31);
    _mpu.setZGyroOffset(-49);

    this->reset();

    DEBUG_LOG(F("HeadingCalculator::HeadingCalculator: MPU initialized."));
}

HeadingCalculator::~HeadingCalculator()
{
}

void HeadingCalculator::reset(int init_heading)
{
    _heading = init_heading;
    _lastUpdate = millis();
}

float HeadingCalculator::update()
{
    unsigned long now = millis();
    if (now > _lastUpdate + HEADING_CALCULATOR_UPDATE_INTERVAL_MILLIS) {
        double gyro_z = _mpu.getRotationZ();
        double gyro_rate = gyro_z / GYRO_LSB_PER_DPS;
        double gyro_angle = gyro_rate * (now - _lastUpdate) / 1000.0;

        _heading += gyro_angle;
        _lastUpdate = now;

        if (_heading > 180) {
            _heading -= 360;
        }
        else if (_heading < -180) {
            _heading += 360;
        }
    }

    return _heading;
}


