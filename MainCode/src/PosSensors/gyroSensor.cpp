#include <PosSensors/gyroSensor.h>
#include <Util/utility.h>

// ----------------------------------------------------------------------------------------------------
// Quaternion class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

float Quaternion::calcRoll() {
    double sinr_cosp = 2 * (r * i + j * k);
    double cosr_cosp = 1 - 2 * (i * i + j * j);
    return atan2(sinr_cosp, cosr_cosp) * RAD_TO_DEG;
}

float Quaternion::calcPitch() {
    double sinp = sqrt(1 + 2 * (r * j - i * k));
    double cosp = sqrt(1 - 2 * (r * j - i * k));
    return (2 * atan2(sinp, cosp) - M_PI / 2) * RAD_TO_DEG;
}

float Quaternion::calcYaw() {
    double siny_cosp = 2 * (r * k + i * j);
    double cosy_cosp = 1 - 2 * (j * j + k * k);
    return atan2(siny_cosp, cosy_cosp) * RAD_TO_DEG;
}

// ----------------------------------------------------------------------------------------------------
// GyroSensor class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

float GyroSensor::getYaw() {
    if (_upd_yaw)
        return _yaw;
    _yaw = wrap360(_lastReading.calcYaw());
    _upd_yaw = true;
    return _yaw;
}

float GyroSensor::getPitch() {
    if (_upd_pitch)
        return _pitch;
    _pitch = wrap360(_lastReading.calcPitch());
    _upd_pitch = true;
    return _pitch;
}

float GyroSensor::getRoll() {
    if (_upd_roll)
        return _roll;
    _roll = wrap360(_lastReading.calcRoll());
    _upd_roll = true;
    return _roll;
}

bool GyroSensor::begin() {
    if (!sensor.begin_I2C()) {
        DB_COLOR_PRINTLN(F("FAILED TO BEGIN GYRO SENSOR!"), SET_RED);
        return false;
    }
    if (!sensor.enableReport(SH2_GAME_ROTATION_VECTOR)) {
        DB_COLOR_PRINTLN(F("COULD NOT ENABLE ROTATION VECTOR REPORT!"), SET_RED);
        return false;
    }
    DB_PRINTLN(F("Successfully begun gyro sensor!"));
    return true;
}

void GyroSensor::update() {
    if (!sensor.getSensorEvent(&_sensorValue))
        return;
    if (_sensorValue.sensorId != SH2_GAME_ROTATION_VECTOR) //SH2_ROTATION_VECTOR)
        return;
    _lastReading.r = _sensorValue.un.gameRotationVector.real;
    _lastReading.i = _sensorValue.un.gameRotationVector.i;
    _lastReading.j = _sensorValue.un.gameRotationVector.j;
    _lastReading.k = _sensorValue.un.gameRotationVector.k;
    _upd_yaw = false;
    _upd_pitch = false;
    _upd_roll = false;
}