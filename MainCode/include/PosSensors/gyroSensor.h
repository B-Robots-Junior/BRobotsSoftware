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

//! READ THIS WHEN COPYING OVER OR CHANGING SOMETHING
/*
    Because we only need SH2_GAME_ROTATION_VECTOR for this class and
    the current shtp implementation from adafruit takes way too much ram (a wopping 1.9 kBytes),
    because of the static instances variable in .pio/libdeps/megaatmega2560/Adafruit BNO08x/shtp.c,
    we can simply change some constants for communication buffers:

    1. change: 
        #define SH2_MAX_APPS (5) -> #define SH2_MAX_APPS (3)
        decided through trial and error
        in shtp.c line 31
    
    2. change:
        #define SH2_MAX_CHANS (8) -> #define SH2_MAX_CHANS (4)
        decided through trial and error
        in shtp.c line 33
    
    3. change:
        #define SH2_HAL_MAX_PAYLOAD_IN   (384) -> #define SH2_HAL_MAX_PAYLOAD_IN   (328)
        in sh2_hal.h line 36
        decided through trial and error
    
    4. change:
        #define SH2_HAL_MAX_TRANSFER_OUT (256) -> #define SH2_HAL_MAX_TRANSFER_OUT (32)
        in sh2_hal.h line 29
        decided through trial and error
    
    5. change:
        #define SH2_HAL_MAX_TRANSFER_IN  (384) -> #define SH2_HAL_MAX_TRANSFER_IN  (328)
        also in sh2_hal.h line 35
        decided through trial and error
    
    6. change:
        #define SH2_HAL_MAX_PAYLOAD_OUT  (256) -> #define SH2_HAL_MAX_PAYLOAD_OUT  (32)
        in sh2_hal.h line 30
        this is actually not used in instances, but fuck it its used somewhere so down you go
        decided through trial and error
*/

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