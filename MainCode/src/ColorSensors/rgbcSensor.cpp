#include <ColorSensors/rgbcSensor.h>

// ----------------------------------------------------------------------------------------------------
// RGBCSensor class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methoden
// --------------------------------------------------

void RGBCSensor::setIntPin(uint8_t intPin) {
    detachPinChangeInterrupt(digitalPinToPCINT(_intPin));
    pinMode(_intPin, INPUT);
    _intPin = intPin;
    pinMode(_intPin, INPUT_PULLUP);
    if (_onBlackTile)
        attachPinChangeInterrupt(digitalPinToPCINT(_intPin), onExitInt, FALLING);
    else
        attachPinChangeInterrupt(digitalPinToPCINT(_intPin), onEnterInt, FALLING);
}

bool RGBCSensor::begin(SoftwareWire* theWire, uint8_t addr) {
    DB_PRINTLN(F("Trying to connect to the RGBCSensor!"));

    if (sensor.begin(addr, theWire)) {
        DB_PRINTLN(F("Successfully connected to the RGBCSensor!"));
        _onBlackTile = false;
        pinMode(_intPin, INPUT_PULLUP);
        attachPinChangeInterrupt(digitalPinToPCINT(_intPin), onEnterInt, FALLING);
        updateInterruptTriggerLevel();
        setPersistence(PERSISTENCE_1);
        sensor.setInterrupt(true);
        sensor.clearInterrupt();
        _connectionGood = true;
        return true;
    }
    else {
        DB_PRINT(SET_RED);
        DB_PRINT(F("FAILED TO CONNECT TO THE RGBCSENSOR!"));
        DB_PRINTLN(RESET_COLOR);
        return false;
    }
}

bool RGBCSensor::good() {
    return _connectionGood;
}

void RGBCSensor::saveColorsToEEPROM() {
    for (int f = 0; f < 5; f++)
        saveColorToEEPROM(ColorType(f));
}

void RGBCSensor::loadColorsFromEEPROM() {
    for (int8_t f = 0; f < 5; f++)
       loadColorFromEEPROM(ColorType(f));
}

void RGBCSensor::loadColorFromEEPROM(ColorType color) {
    if (color <= ColorType::Invalid || color > ColorType::Red)
        return;
    for (int c = 0; c < 4; c++)
        colors[static_cast<int8_t>(color)][c] = readFromEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(color) * 4 + c) * sizeof(uint16_t));
    if (color == ColorType::Black)
        updateInterruptTriggerLevel();
}

void RGBCSensor::saveColorToEEPROM(ColorType color) {
     if (color <= ColorType::Invalid || color > ColorType::Red)
        return;
    for (int c = 0; c < 4; c++)
        saveToEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(color) * 4 + c) * sizeof(uint16_t), colors[static_cast<int8_t>(color)][c]);
}

void RGBCSensor::setColor(ColorType color) {
     if (color <= ColorType::Invalid || color > ColorType::Red)
        return;
    if (!_connectionGood)
        return;
    
    sensor.getRawData(&colors[static_cast<int8_t>(color)][0],
                      &colors[static_cast<int8_t>(color)][1],
                      &colors[static_cast<int8_t>(color)][2],
                      &colors[static_cast<int8_t>(color)][3]);
    if (color == ColorType::Black)
        updateInterruptTriggerLevel();
}

ColorType RGBCSensor::getCurrentId() {
    if (!_connectionGood)
        return ColorType::Invalid;
    
    uint16_t r, g, b, c;
    sensor.getRawData(&r, &g, &b, &c);

    int id = 0;
    float minDist = _dist(0, r, g, b, c);

    for (int i = 1; i < 5; i++) {
        float dist = _dist(i, r, g, b, c);
        if (dist < minDist) {
            id = i;
            minDist = dist;
        }
    }

    return ColorType(id);
}

void RGBCSensor::setPersistence(uint8_t persistence) {
    uint8_t lastValue = sensor.read8(PERSISTENCE_FILTER_REGISTER);
    // the last four bytes are reserved so its masked so its unaltered
    sensor.write8(PERSISTENCE_FILTER_REGISTER, (lastValue & 0xF0) | (persistence & 0x0F));
}

void RGBCSensor::clearInterrupt() {
    if (!_connectionGood)
        return;

    sensor.clearInterrupt();
}

void RGBCSensor::updateInterruptTriggerLevel() {
    if (!_connectionGood)
        return;

    _blackIntTriggerLevel = (uint16_t)(colors[RGBC_BLACK_TILE_ID][3] * CLEAR_TOLERANZ + ADDATIVE_CLEAR_TOLERANZ);
    DB_PRINT(F("Set limit to: "));
    DB_PRINTLN(_blackIntTriggerLevel);
    if (_onBlackTile)
        sensor.setIntLimits(0, _blackIntTriggerLevel); // should trigger when exiting
    else
        sensor.setIntLimits(_blackIntTriggerLevel, 0xFFFF); // should trigger when entering
}

void RGBCSensor::enterBlackTile() {
    _onBlackTile = true;
    attachPinChangeInterrupt(digitalPinToPCINT(_intPin), onExitInt, FALLING);
    updateInterruptTriggerLevel();
    clearInterrupt();
}

void RGBCSensor::exitBlackTile() {
    _onBlackTile = false;
    attachPinChangeInterrupt(digitalPinToPCINT(_intPin), onEnterInt, FALLING);
    updateInterruptTriggerLevel();
    clearInterrupt();
}

// --------------------------------------------------
// private methoden
// --------------------------------------------------

float RGBCSensor::_dist(int tileId, uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    return sqrtf(pow(colors[tileId][0] - r, 2) + pow(colors[tileId][1] - g, 2) + pow(colors[tileId][2] - b, 2) + pow(colors[tileId][3] - c, 2));
}