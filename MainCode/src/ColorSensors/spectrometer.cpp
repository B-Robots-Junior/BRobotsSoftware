#include <ColorSensors/spectrometer.h>

// ----------------------------------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------------------------------

const char* specIdToAbrv[] = {
    "norm",
    "blue",
    "blck",
    "chck",
    "red"
};

const char* specIdToName[] = {
    "normal",
    "blue",
    "black",
    "checkpoint",
    "red"
};

// ----------------------------------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------------------------------

bool weightedCheck(uint16_t value, uint16_t lower, uint16_t upper, float weight) {
    return value >= lower + (upper - lower) * weight;
}

// ----------------------------------------------------------------------------------------------------
// Spectrometer class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

bool Spectrometer::begin(uint8_t i2c_addr, TwoWire *wire, int32_t sensor_id) {
    _good = sensor.begin(i2c_addr, wire, sensor_id);
    if (!_good)
        return false;
    sensor.enableLED(true);
    sensor.setASTEP(ASTEP_RECOMENDED);
    sensor.setATIME(ATIME_RECOMENDED);
    sensor.startReading();
    _readingStartTime = millis();
    return _good;
}

void Spectrometer::update() {
    if (!_good)
        return;
    
    bool readingFinished = sensor.checkReadingProgress();
    if (!readingFinished) {
        // no data available check timeout
        if (millis() - _readingStartTime >= _readingComplTimeout) {
            sensor.startReading();
            _readingStartTime = millis();
        }
        return;
    }
    // data available, get data and start new reading
    sensor.getAllChannels(currentReading);
    _newReading = true; // mark there is a new reading
    sensor.startReading();
    _readingStartTime = millis();
}

void Spectrometer::saveColorToEEPROM(uint8_t colorId) {
    if (colorId >= SPEC_NUM_IDS)
        return;
    for (uint8_t c = 0; c < 12; c++)
        saveToEEPROM<uint16_t>(_eepromOffset + (colorId * 12 + c) * sizeof(uint16_t), colors[colorId][c]);
}

void Spectrometer::readColorFromEEPROM(uint8_t colorId) {
    if (colorId >= SPEC_NUM_IDS)
        return;
    for (uint8_t c = 0; c < 12; c++)
        colors[colorId][c] = readFromEEPROM<uint16_t>(_eepromOffset + (colorId * 12 + c) * sizeof(uint16_t));
}

void Spectrometer::saveColorsToEEPROM() {
    for (uint8_t colorId = 0; colorId < SPEC_NUM_IDS; colorId++)
        saveColorToEEPROM(colorId);
}

void Spectrometer::readColorsFromEEPROM() {
    for (uint8_t colorId = 0; colorId < SPEC_NUM_IDS; colorId++)
        readColorFromEEPROM(colorId);
}

void Spectrometer::setColorCurrent(uint8_t colorId) {
    if (!_good || colorId >= SPEC_NUM_IDS)
        return;
    
    uint32_t sum[12] = {0};
    for (uint8_t i = 0; i < SPEC_NUM_SAMPLE_READINGS; i++) {
        sensor.readAllChannels(currentReading);
        for (uint8_t c = 0; c < 12; c++) {
            sum[c] += currentReading[c];
        }
    }
    for (uint8_t c = 0; c < 12; c++) {
        colors[colorId][c] = sum[c] / SPEC_NUM_SAMPLE_READINGS;
    }
    saveColorToEEPROM(colorId); // save the updated color

    _newReading = true;
    sensor.startReading(); // start a new reading again, so the coninuous reading is not broken
    _readingStartTime = millis();
}

int8_t Spectrometer::getColorId() {
    if (!_good)
        return SPEC_INVALID_TILE_ID;
    
    if (!_newReading) {
        return _currentColorId;
    }
    // check if it is eigher a bright or a dark color
    bool isLightColor = weightedCheck(currentReading[specCha::CLEAR_0], colors[SPEC_BLUE_TILE_ID][specCha::CLEAR_0], colors[SPEC_NORMAL_TILE_ID][specCha::CLEAR_0], DARK_BRIGHT_DIFF_WEIGHTS);
    if (isLightColor) {
        // through a bit of experementing the only seeable difference is how inconsistent the values of the checkpoit tiles are
        // so I'm just gonna calculate the error of the is value to the normal tile value and if that is over a threshold it is a checkpoint
        // its haveing a few troubles with speed bumps, maybe its too dark when the light is seperated
        // if the reading is consistently under the calibrated value it is moast likely a speed bump with a normal tile

        uint32_t err = 0; // sum of the channel errors
        bool onlyBelow = true;
        for (uint8_t c = 0; c < specCha::CLEAR; c++) {
            if (c == specCha::CLEAR_0 || c == specCha::NIR_0)
                continue;
            int32_t diff = (int32_t)currentReading[c] - (int32_t)colors[SPEC_NORMAL_TILE_ID][c];
            onlyBelow = onlyBelow && diff < 0;
            err += abs(diff);
        }
        if (err >= CHEC_ERROR_THRESHOLD && !onlyBelow)
            _currentColorId = SPEC_CHECK_TILE_ID;
        else
            _currentColorId = SPEC_NORMAL_TILE_ID;

        // // the channels F3 an F5 are quite a bit higher on checkpoint tiles then they are on normal tiles
        // // so I'm taking a bit of a weighted toleranz, because normal tiles are more likely to be measured
        // bool aboveF3 = weightedCheck(
        //     currentReading[specCha::F3_480nm], 
        //     colors[SPEC_NORMAL_TILE_ID][specCha::F3_480nm], 
        //     colors[SPEC_CHECK_TILE_ID][specCha::F3_480nm], 
        // NORMAL_CHECK_DIFF_WEIGHTS);
        // bool aboveF5 = weightedCheck(
        //     currentReading[specCha::F5_555nm], 
        //     colors[SPEC_NORMAL_TILE_ID][specCha::F5_555nm], 
        //     colors[SPEC_CHECK_TILE_ID][specCha::F5_555nm], 
        // NORMAL_CHECK_DIFF_WEIGHTS);
        // if (aboveF3 && aboveF5)
        //     _currentColorId = SPEC_CHECK_TILE_ID;
        // else
        //     _currentColorId = SPEC_NORMAL_TILE_ID;
    }
    else {
        // the highest difference between black and blue is in the clear channel and F3 480nm / F2 445nm, becuase around 450nm is the main blue spectrum
        bool clearValid = weightedCheck(
            currentReading[specCha::CLEAR_0], 
            colors[SPEC_BLACK_TILE_ID][specCha::CLEAR_0], 
            colors[SPEC_BLUE_TILE_ID][specCha::CLEAR_0],
        BLACK_BLUE_DIFF_WEIGHTS);
        bool aboveF3 = weightedCheck(
            currentReading[specCha::F3_480nm], 
            colors[SPEC_BLACK_TILE_ID][specCha::F3_480nm], 
            colors[SPEC_BLUE_TILE_ID][specCha::F3_480nm],
        BLACK_BLUE_DIFF_WEIGHTS);
        bool aboveF2 = weightedCheck(
            currentReading[specCha::F2_445nm], 
            colors[SPEC_BLACK_TILE_ID][specCha::F2_445nm], 
            colors[SPEC_BLUE_TILE_ID][specCha::F2_445nm],
        BLACK_BLUE_DIFF_WEIGHTS);

        bool aboveF6 = weightedCheck(
            currentReading[specCha::F6_590nm],
            colors[SPEC_BLUE_TILE_ID][specCha::F6_590nm],
            colors[SPEC_RED_TILE_ID][specCha::F6_590nm],
        BLUE_RED_DIFF_WEIGHTS);
        bool aboveF7 = weightedCheck(
            currentReading[specCha::F7_630nm],
            colors[SPEC_BLUE_TILE_ID][specCha::F7_630nm],
            colors[SPEC_RED_TILE_ID][specCha::F7_630nm],
        BLUE_RED_DIFF_WEIGHTS);
        bool aboveF8 = weightedCheck(
            currentReading[specCha::F8_680nm],
            colors[SPEC_BLUE_TILE_ID][specCha::F8_680nm],
            colors[SPEC_RED_TILE_ID][specCha::F8_680nm],
        BLUE_RED_DIFF_WEIGHTS);

        if (aboveF6 && aboveF7 && aboveF8)
            _currentColorId = SPEC_RED_TILE_ID;
        else if (clearValid && (aboveF2 || aboveF3))
            _currentColorId = SPEC_BLUE_TILE_ID;
        else
            _currentColorId = SPEC_BLACK_TILE_ID;
    }

    _newReading = false;
    return _currentColorId;
}