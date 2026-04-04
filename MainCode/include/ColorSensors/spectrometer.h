#ifndef _SPECTROMETER_H_
#define _SPECTROMETER_H_

#include <Adafruit_AS7341.h>
#include <debug.h>
#include <Util/eepromUtil.h>
#include <config.h>
#include <ColorSensors/colorType.h>

#define SPEC_DEFAULT_READING_COMPL_TIMEOUT 1000

#define NORMAL_CHECK_DIFF_WEIGHTS 0.66 // this means the lower 66% of the range distance between normal and checkpoint tiles counts to normal
#define DARK_BRIGHT_DIFF_WEIGHTS 0.5 // this means the clear channel has to reach over the avarige of blue and normal to count as bright
#define BLACK_BLUE_DIFF_WEIGHTS 0.3 // the lower 30% of the range differnece for the clear is black the top 70% is blue (because black is more consistently dark)
#define BLUE_RED_DIFF_WEIGHTS 0.5 // the upper 50% of the range difference for F6, F7, F8 are for red tiles

#define CHEC_ERROR_THRESHOLD 1500 // the error threshold from a normal tile at which the tile is considered a checkpoint

#define SPEC_NUM_SAMPLE_READINGS 10 // the number of readings for a color that get averaged

// 50 ms integration recomended by the the datasheet
#define ASTEP_RECOMENDED 599
#define ATIME_RECOMENDED 29

enum specCha {
    F1_415nm,
    F2_445nm,
    F3_480nm,
    F4_515nm,
    CLEAR_0,
    NIR_0,
    F5_555nm,
    F6_590nm,
    F7_630nm,
    F8_680nm,
    CLEAR,
    NIR,
};

bool weightedCheck(uint16_t value, uint16_t lower, uint16_t upper, float weight); // true for above, false for below

class Spectrometer {
private:
    uint32_t _eepromOffset;
    bool _good = false;
    uint64_t _readingStartTime = 0;
    uint32_t _readingComplTimeout = 0;
    bool _newReading = true;
    ColorType _currentColorId = ColorType::Invalid;

public:
    Spectrometer(uint32_t readingComplTimeout = SPEC_DEFAULT_READING_COMPL_TIMEOUT, uint32_t eepromOffset = SPECTROMETER_OFFSET_EEPROM) 
        : _eepromOffset(eepromOffset), _readingComplTimeout(readingComplTimeout) { readColorsFromEEPROM(); }

    uint16_t colors[5][12];
    uint16_t currentReading[12];

    Adafruit_AS7341 sensor;

    bool good() const {return _good;}

    bool begin(uint8_t i2c_addr = (uint8_t)57U, TwoWire *wire = &Wire, int32_t sensor_id = 0);
    void update();
    void forceUpdate();

    void saveColorToEEPROM(ColorType colorId);
    void readColorFromEEPROM(ColorType colorId);
    void saveColorsToEEPROM();
    void readColorsFromEEPROM();
    void setColorCurrent(ColorType colorId);

    ColorType getColorId();
};

#endif