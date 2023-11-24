#include <Arduino.h>
#include <Wire.h>
#include "HeadingCalculator.h"
#include "DataLogger.h"


const unsigned long HEADING_CALCULATOR_UPDATE_INTERVAL_MILLIS = 50;

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
    _mpu.setFullScaleGyroRange(MPU6050_IMU::GYRO_FS::MPU6050_GYRO_FS_250);

    // set the mpu6050 offsets. These were determined by running the calibration code in the
    // Arduino C++ library and then converting the results to Rust. The calibration code is here:
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
    if (millis() > _lastUpdate + HEADING_CALCULATOR_UPDATE_INTERVAL_MILLIS) {
        _lastUpdate = millis();

        int16_t ax, ay, az;
        int16_t gx, gy, gz;
        _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        float accel_angle = atan2(ay, az) * 180 / M_PI;
        float gyro_rate = gx / 131.0;
        float gyro_angle = gyro_rate * (millis() - _lastUpdate) / 1000.0;

        _heading += gyro_angle;
        _heading = 0.98 * (_heading + gyro_angle) + 0.02 * accel_angle;
    }
}


