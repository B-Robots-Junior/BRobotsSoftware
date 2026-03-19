#ifndef REF_SENSOR_H
#define REF_SENSOR_H

#include <Arduino.h>
#include <debug.h>

// pins:
// white: C (CTRL / LED)
// blue: S (sensor output)

enum class ColorType : int8_t {
    INVALID = -1,
    NORMAL = 0,
    BLUE = 1,
    BLACK = 2,
    CHECKPOINT = 3,
    RED = 4
};

class RefSensor {
public:
    RefSensor(uint8_t ledPin, uint8_t inputPin) : _ledPin(ledPin), _inputPin(inputPin) {}

    bool begin(); // returns true is initialized sucessfully
    void calibrate(ColorType type); // calibrates the current type
    bool isCheckpoint(); // checks is the current tile is a checkpoint
    uint16_t read(); // get a raw reading from the sensor

private:
    uint16_t _ncMinRef = 0xFFFF;
    uint16_t _cMaxRef = 0x0000;

    uint8_t _ledPin;
    uint8_t _inputPin;
};

bool RefSensor::begin() {
    pinMode(_ledPin, OUTPUT);
    pinMode(_inputPin, INPUT);
    digitalWrite(_ledPin, HIGH);
    return true;
}

void RefSensor::calibrate(ColorType type) {
    if (type == ColorType::CHECKPOINT) {

        return;
    }
}

uint16_t RefSensor::read() {
    return analogRead(_inputPin);
    /*
    digitalWrite(_ledPin, LOW);
    delay(2);
    uint16_t ambient = analogRead(_inputPin);

    digitalWrite(_ledPin, HIGH);
    delay(2);
    uint16_t withLED = analogRead(_inputPin);
    digitalWrite(_ledPin, LOW);

    if (withLED > ambient)
        return 0;
    return ambient - withLED;
    */
}

#endif