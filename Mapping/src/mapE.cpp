#include <mapE.h>

// ----------------------------------------------------------------------------------------------------
// TileCon class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// Constructers
// --------------------------------------------------

TileCon::TileCon(uint8_t data) : 
        adjacentTiles((data >> ADJACENT_TILES_BIT) & 1),
        type(data >> TILE_TYPE_START_BIT) 
{
    if (type == RAMP_TILE) {
        rampDir = (data >> RAMP_DIR_START_BIT) & 0b11;
        up = (data >> RAMP_UP_BIT) & 1;
    }
    else {
        north = (data >> NORTH_WALL_BIT) & 1;
        east = (data >> EAST_WALL_BIT) & 1;
        south = (data >> SOUTH_WALL_BIT) & 1;
        west = (data >> WEST_WALL_BIT) & 1;
    }
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

void TileCon::print() const {
    #if DEBUG_MODE
    String empty = type_atering[type] + " " + RESET_COLOR;
    if (type == RAMP_TILE) {
        DB_PRINTLN(((rampDir != 2) ? "###" : "#" + empty + "#"));
        DB_PRINT(((rampDir != 1) ? "#" + empty : empty + empty)); DB_PRINTLN(((rampDir != 3) ? "#" : empty));
        DB_PRINT(((rampDir != 0) ? "###" : "#" + empty + "#"));
    }
    else {
        DB_PRINTLN((north ? "###" : "#" + empty + "#"));
        DB_PRINT((west ? "#" + empty : empty + empty)); DB_PRINTLN((east ? "#" : empty));
        DB_PRINT((south ? "###" : "#" + empty + "#"));
    }
    #endif
}

void TileCon::println() const {
    print();
    DB_PRINTLN();
}

BitPtr<1> TileCon::operator[](int n) {
    if (type == RAMP_TILE) {
        switch (n) {
        case RAMP_DIR_START_BIT: return BitPtr<1>((uint8_t*)&rampDir, 0); 
        case RAMP_DIR_START_BIT + 1: return BitPtr<1>((uint8_t*)&rampDir, 1);
        case RAMP_UP_BIT: return BitPtr<1>((uint8_t*)&up, 0);
        }
    }
    switch (n) {
    case NORTH_WALL_BIT: return BitPtr<1>((uint8_t*)&north, 0);
    case EAST_WALL_BIT: return BitPtr<1>((uint8_t*)&east, 0);
    case WEST_WALL_BIT: return BitPtr<1>((uint8_t*)&west, 0);
    case SOUTH_WALL_BIT: return BitPtr<1>((uint8_t*)&south, 0);
    case ADJACENT_TILES_BIT: return BitPtr<1>((uint8_t*)&adjacentTiles, 0);
    case TILE_TYPE_START_BIT: return BitPtr<1>((uint8_t*)&type, 0);
    case TILE_TYPE_START_BIT + 1: return BitPtr<1>((uint8_t*)&type, 1);
    case TILE_TYPE_START_BIT + 2: return BitPtr<1>((uint8_t*)&type, 2);
    }
    return BitPtr<1>((uint8_t*)&north, 0);
}

TileCon::operator uint8_t() {
    if (type == RAMP_TILE)
        return RTC(rampDir, up, adjacentTiles, type);
    return TC(north, east, south, west, adjacentTiles, type);
}

// ----------------------------------------------------------------------------------------------------
// MapE class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

uint32_t MapE::dynamicMemorySize() const {
    return _layers * sizeof(struct Layer);
}

uint32_t MapE::eepromMemorySize() const {
    uint32_t sum = 0;
    for (uint8_t z = 0; z < _layers; z++)
        sum += _data[z].width * _data[z].height;
    return sum;
}

uint8_t MapE::get(int8_t x, int8_t y, int8_t z) const {
    int8_t rz = z + _offsetZ;
    if (rz < 0 || rz >= _layers)
        return _defData;
    struct Layer* currLayer = &_data[rz];
    int8_t rx = x + currLayer->offsetX;
    int8_t ry = y + currLayer->offsetY;
    if (rx < 0 || rx >= currLayer->width || ry < 0 || ry >= currLayer->height)
        return _defData;
    if (currLayer->index == EE_NULLPTR)
        return _defData;
    return EEPROM.read(currLayer->index + ry * currLayer->width + rx);
}

bool MapE::getBit(int8_t x, int8_t y, int8_t z, uint8_t bit) const {
    return (get(x, y, z) >> bit) & 1;
}

bool MapE::getWall(int8_t x, int8_t y, int8_t z, uint8_t direc) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != ((direc + 2) % 4);
    else
        return (data >> (direc % 4)) & 1;
}

bool MapE::getNorth(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 2;
    else
        return _GET_NORTH(data) & 1;
}

bool MapE::getEast(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 3;
    else
        return _GET_EAST(data) & 1;
}

bool MapE::getSouth(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 0;
    else
        return _GET_SOUTH(data) & 1;
}

bool MapE::getWest(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 1;
    else
        return _GET_WEST(data) & 1;
}

void MapE::set(int8_t x, int8_t y, int8_t z, uint8_t data) {
    // initial expansion when there is no data
    if (_layers == 0) {
        _data = (struct Layer*)malloc(sizeof(struct Layer));
        _data[0] = {EE_NULLPTR, 0, 0, 0, 0};
        _layers = 1;
        _offsetZ = -z;
    }

    int8_t rz = z + _offsetZ;

    // check and expand the x direction
    if (rz < 0) {
        uint8_t amount = abs(rz);
        struct Layer* newData = (struct Layer*)malloc(sizeof(struct Layer) * (_layers + amount));
        memcpy(&newData[amount], _data, sizeof(struct Layer) * _layers);
        for (uint8_t i = 0; i < amount; i++) {
            newData[i] = {EE_NULLPTR, 0, 0, 0, 0};
        }
        free(_data);
        _data = newData;
        _layers += amount;
        _offsetZ += amount;
        rz += amount;
    }
    else if (rz >= _layers) {
        uint8_t amount = rz - _layers + 1;
        struct Layer* newData = (struct Layer*)malloc(sizeof(struct Layer) * (_layers + amount));
        memcpy(newData, _data, sizeof(struct Layer) * _layers);
        for (uint8_t i = _layers; i < _layers + amount; i++) {
            newData[i] = {EE_NULLPTR, 0, 0, 0, 0};
        }
        free(_data);
        _data = newData;
        _layers += amount;
    }

    struct Layer* currLayer = &_data[rz];

    // initial allocation when no data existing
    if (currLayer->width == 0 || currLayer->height == 0 || currLayer->index == EE_NULLPTR) {
        currLayer->index = _elloc(1);
        if (currLayer->index == EE_NULLPTR) //! dangerous, because it fails silently
            return;
        currLayer->width = 1;
        currLayer->height = 1;
        currLayer->offsetX = -x;
        currLayer->offsetY = -y;
    }

    int8_t rx = x + currLayer->offsetX;
    int8_t ry = y + currLayer->offsetY;

    // check needed expansion amount
    uint8_t amountX = 0;
    uint8_t amountY = 0;
    uint8_t oldDataOffsetX = 0;
    uint8_t oldDataOffsetY = 0;
    if (rx < 0) {
        amountX = abs(rx);
        oldDataOffsetX = amountX;
    }
    else if (rx >= currLayer->width)
        amountX = rx - currLayer->width + 1;
    if (ry < 0) {
        amountY = abs(ry);
        oldDataOffsetY = amountY;
    }
    else if (ry >= currLayer->height)
        amountY = ry - currLayer->height + 1;
    
    // expand data by needed amount
    if (amountX != 0 || amountY != 0) {
        uint8_t newWidth = currLayer->width + amountX;
        uint8_t newHeight = currLayer->height + amountY;
        uint16_t newIndex = _elloc(newWidth * newHeight);
        if (newIndex == EE_NULLPTR) //! this is dangerous because it fails silently
            return;

        // bottom padding
        for (uint8_t iy = 0; iy < oldDataOffsetY; iy++) {
            for (uint8_t ix = 0; ix < newWidth; ix++)
                EEPROM.write(newIndex + iy * newWidth + ix, _defData);
        }    
        for (uint8_t iy = 0; iy < currLayer->height; iy++) {
            // data copying
            for (uint8_t ix = 0; ix < currLayer->width; ix++) {
                EEPROM.write(newIndex + (iy + oldDataOffsetY) * newWidth + (ix + oldDataOffsetX), 
                    EEPROM.read(currLayer->index + iy * currLayer->width + ix));
            }
            // left padding
            for (uint8_t ix = 0; ix < oldDataOffsetX; ix++)
                EEPROM.write(newIndex + (iy + oldDataOffsetY) * newWidth + ix, _defData);
            // right padding
            for (uint8_t ix = currLayer->width + oldDataOffsetX; ix < newWidth; ix++)
                EEPROM.write(newIndex + (iy + oldDataOffsetY) * newWidth + ix, _defData);
        }
        // top padding
        for (uint8_t iy = currLayer->height + oldDataOffsetY; iy < newHeight; iy++) {
            for (uint8_t ix = 0; ix < newWidth; ix++)
                EEPROM.write(newIndex + iy * newWidth + ix, _defData);
        }

        currLayer->index = newIndex;
        currLayer->width = newWidth;
        currLayer->height = newHeight;
        currLayer->offsetX += oldDataOffsetX;
        currLayer->offsetY += oldDataOffsetY;
        rx += oldDataOffsetX;
        ry += oldDataOffsetY;
    }

    // final expanded data writing
    EEPROM.write(currLayer->index + ry * currLayer->width + rx, data);
}

void MapE::setBit(int8_t x, int8_t y, int8_t z, uint8_t bit, bool state) {
    uint8_t data = get(x, y, z);
    if (state)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    set(x, y, z, data);
}

void MapE::setType(int8_t x, int8_t y, int8_t z, uint8_t type) {
    uint8_t data = get(x, y, z) & 0b00011111;
    data |= type << TILE_TYPE_START_BIT;
    set(x, y, z, data);
}

void MapE::setRampDir(int8_t x, int8_t y, int8_t z, uint8_t dir) {
    uint8_t data = get(x, y, z) & 0b11111100;
    data |= dir & 0b11;
    set(x, y, z, data);
}

void MapE::print(int8_t x, int8_t y, int8_t z) const {
    #if DEBUG_MODE
    String empty = type_atering[getType(x, y, z)] + " " + RESET_COLOR;
    DB_PRINTLN((getNorth(x, y, z) ? "###" : "#" + empty + "#"));
    DB_PRINT((getWest(x, y, z) ? "#" + empty : empty + empty)); DB_PRINTLN((getEast(x, y, z) ? "#" : empty));
    DB_PRINT((getSouth(x, y, z) ? "###" : "#" + empty + "#"));
    #endif
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

uint16_t MapE::_elloc(uint32_t size) {
    uint16_t currIndex = _startIndex;
    bool collides = false;
    while (true) {
        collides = false;
        uint16_t collidesEndIndex = EE_NULLPTR;
        if (currIndex + size > _endIndex) {
            ERROR_MINOR("RAN OUT OF EEPROM MEMORY!", SET_RED);
            return EE_NULLPTR; // no more space
        }
        for (uint8_t i = 0; i < _layers; i++) {
            if (_data[i].index < currIndex) {
                uint16_t endIndex = _data[i].index + _data[i].width * _data[i].height;
                if (endIndex > currIndex) {
                    collides = true;
                    collidesEndIndex = endIndex;
                    break;
                }
            }
            else {
                if (currIndex + size > _data[i].index) {
                    collides = true;
                    collidesEndIndex = _data[i].index + _data[i].width * _data[i].height;
                    break;
                }
            }
        }
        if (!collides)
            return currIndex;
        currIndex = collidesEndIndex;
    }
}