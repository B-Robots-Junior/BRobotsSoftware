#include <posSensors/gyroSensor.h>

// ----------------------------------------------------------------------------------------------------
// GyroSensor class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

float GyroSensor::getYaw() {
    return _lastReading.x();
}

float GyroSensor::getPitch() {
    return _lastReading.y();
}

float GyroSensor::getRoll() {
    return _lastReading.z();
}

bool GyroSensor::begin() {
    // adafruit_bno055_opmode_t::OPERATION_MODE_NDOF
    if (!sensor.begin(adafruit_bno055_opmode_t::OPERATION_MODE_NDOF)) { // start in config mode to enable interrupts
        ERROR_MINOR(F("Could not begin the BNO055"), SET_RED);
        return false;
    }

    delay(25);

    return true;
}

void GyroSensor::update() {
    _lastReading = sensor.getVector(Adafruit_BNO055::VECTOR_EULER);
}