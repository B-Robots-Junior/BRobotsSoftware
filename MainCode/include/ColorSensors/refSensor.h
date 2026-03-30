#ifndef REF_SENSOR_H
#define REF_SENSOR_H

#include <Arduino.h>
#include <debug.h>
#include <ColorSensors/colorType.h>
#include <config.h>
#include <Util/eepromUtil.h>

// pins:
// white: C (CTRL / LED)
// blue: S (sensor output)

#define REF_SENSOR_CALIB_AMOUNT 100

class RefSensor {
public:
    RefSensor(uint8_t ledPin, uint8_t inputPin, uint32_t eepromOffset = REF_SENSOR_OFFSET_EEPROM) 
        : _ledPin(ledPin), _inputPin(inputPin), _eepromOffset(eepromOffset) {}

    bool begin(); // returns true is initialized sucessfully
    void calibrate(ColorType type); // calibrates the current type
    ColorType getType(); // gets the current type
    uint16_t read(); // get a raw reading from the sensor
    void saveColorToEEPROM(ColorType type);
    void loadColorFromEEPROM(ColorType type);
    void saveColorsToEEPROM();
    void loadColorsFromEEPROM();

private:
    uint16_t _calVals[5] = {0, };

    uint8_t _ledPin;
    uint8_t _inputPin;
    uint32_t _eepromOffset;
};

#endif