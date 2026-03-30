#include <ColorSensors/refSensor.h>

bool RefSensor::begin() {
    pinMode(_ledPin, OUTPUT);
    pinMode(_inputPin, INPUT);
    digitalWrite(_ledPin, HIGH);
    return true;
}

void RefSensor::calibrate(ColorType type) {
    if (type <= ColorType::Invalid || type > ColorType::Red)
        return;
    uint64_t sum = 0;
    for (int i = 0; i < REF_SENSOR_CALIB_AMOUNT; i++)
        sum += read();
    uint16_t val = sum / REF_SENSOR_CALIB_AMOUNT;
    _calVals[static_cast<int8_t>(type)] = val;
}

uint16_t RefSensor::read() {
    digitalWrite(_ledPin, LOW);
    delayMicroseconds(100);
    uint16_t ambient = analogRead(_inputPin);

    digitalWrite(_ledPin, HIGH);
    delayMicroseconds(100);
    uint16_t withLED = analogRead(_inputPin);
    digitalWrite(_ledPin, LOW);

    if (withLED > ambient)
        return 0;
    return ambient - withLED;
}

ColorType RefSensor::getType() {
    uint16_t minDist = 0xFFFF;
    int8_t minColor = -1;
    uint16_t val = read();
    for (uint8_t i = 0; i < 5; i++) {
        int32_t diff = static_cast<int32_t>(_calVals[i]) - static_cast<int32_t>(val);
        uint32_t abs_diff = abs(diff);
        if (abs_diff < minDist) {
            minDist = abs_diff;
            minColor = i;
        }
    }
    return ColorType(minColor);
}

void RefSensor::saveColorToEEPROM(ColorType type) {
    if (type <= ColorType::Invalid || type > ColorType::Red)
        return;
    saveToEEPROM<uint16_t>(_eepromOffset + static_cast<int8_t>(type) * sizeof(uint16_t), _calVals[static_cast<int8_t>(type)]);
}

void RefSensor::loadColorFromEEPROM(ColorType type) {
    if (type <= ColorType::Invalid || type > ColorType::Red)
        return;
    _calVals[static_cast<int8_t>(type)] = readFromEEPROM<uint16_t>(_eepromOffset + static_cast<int8_t>(type) * sizeof(uint16_t));
}

void RefSensor::saveColorsToEEPROM() {
    for (uint8_t i = 0; i < 5; i++)
        saveColorToEEPROM(ColorType(i));
}

void RefSensor::loadColorsFromEEPROM() {
    for (uint8_t i = 0; i < 5; i++)
        loadColorFromEEPROM(ColorType(i));
}
