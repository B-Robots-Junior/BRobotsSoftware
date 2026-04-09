#ifndef _GYRO_SENSOR_H_
#define _GYRO_SENSOR_H_

#include <Adafruit_BNO055.h>
#include <debug.h>

class GyroSensor {
public:
    GyroSensor() : sensor(-1, 0x28) {}

    Adafruit_BNO055 sensor;

    float getYaw(); // in deg
    float getPitch(); // in deg
    float getRoll(); // in deg

    bool begin(); // returns true if successfull
    void update(); // reads in new sensor values (if available for reading else it just doesn't update the values)

private:
    imu::Vector<3> _lastReading = imu::Vector<3>(0, 0, 0);
};

#endif