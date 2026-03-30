#ifndef _GYRO_SENSOR_H_
#define _GYRO_SENSOR_H_

#include <Adafruit_BNO08x.h>
#include <debug.h>

class Quaternion {
public:
    Quaternion(float r = 1.0, float i = 0.0, float j = 0.0, float k = 0.0) : r(r), i(i), j(j), k(k) {}

    float r, i, j, k;

    float calcRoll(); // in deg
    float calcPitch(); // in deg
    float calcYaw(); // in deg
};

class GyroSensor {
private:
    Quaternion _lastReading;
    sh2_SensorValue_t _sensorValue;

    bool _upd_yaw = false, _upd_pitch = false, _upd_roll = false;
    float _yaw = 0, _pitch = 0, _roll = 0; // in deg

public:
    GyroSensor() : sensor(-1) {}

    Adafruit_BNO08x sensor;

    Quaternion getLastQuatReading() const {return _lastReading;}

    float getYaw(); // in deg
    float getPitch(); // in deg
    float getRoll(); // in deg

    bool begin(); // returns true if successfull
    void update(); // reads in new sensor values (if available for reading else it just doesn't update the values)
};

#endif