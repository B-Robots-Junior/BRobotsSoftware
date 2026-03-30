#include <Mapping/map.h>
#include <panikFlags.h>

// ----------------------------------------------------------------------------------------------------
// TileCon class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// Constructers
// --------------------------------------------------

TileCon::TileCon(uint8_t data) : 
        adjacentTiles((data >> ADJACENT_TILES_BIT) & 1),
        type((data >> TILE_TYPE_START_BIT) & 0b111) 
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
    if (type == RAMP_TILE) {
        DB_PRINTLN(((rampDir != 2) ? F("###") : F("# #")));
        DB_PRINT(((rampDir != 1) ? "# " : "  ")); DB_PRINTLN(((rampDir != 3) ? "#" : " "));
        DB_PRINT(((rampDir != 0) ? F("###") : F("# #")));
    }
    else {
        DB_PRINTLN((north ? F("###") : F("# #")));
        DB_PRINT((west ? "# " : "  ")); DB_PRINTLN((east ? "#" : " "));
        DB_PRINT((south ? F("###") : F("# #")));
    }
    #endif
}

void TileCon::println() const {
    print();
    DB_PRINTLN();
}

BitPtr<1> TileCon::operator[](int8_t n) {
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

TileCon::operator uint8_t() const {
    if (type == RAMP_TILE)
        return RTC(rampDir, up, adjacentTiles, type);
    return TC(north, east, south, west, adjacentTiles, type);
}

// ----------------------------------------------------------------------------------------------------
// Map class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// constructors / destrucotors
// --------------------------------------------------

Map::Map(Map&& other) : _data(other._data), _layers(other._layers), _offsetZ(other._offsetZ), _defData(other._defData) {
    other._data = nullptr;
    other._layers = 0;
    other._offsetZ = 0;
}

Map::~Map() {
    _freeAll();
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

uint32_t Map::dynamicMemorySize() const {
    uint32_t sum = 0;
    for (uint8_t z = 0; z < _layers; z++)
        sum += _data[z].width * _data[z].height;
    return sum + _layers * sizeof(struct Layer);
}

uint8_t Map::get(int8_t x, int8_t y, int8_t z) const {
    int8_t rz = z + _offsetZ;
    if (rz < 0 || rz >= _layers)
        return _defData;
    struct Layer* currLayer = &_data[rz];
    int8_t rx = x + currLayer->offsetX;
    int8_t ry = y + currLayer->offsetY;
    if (rx < 0 || rx >= currLayer->width || ry < 0 || ry >= currLayer->height)
        return _defData;
    if (currLayer->data == nullptr)
        return _defData;
    return currLayer->data[ry * currLayer->width + rx];
}

bool Map::getBit(int8_t x, int8_t y, int8_t z, uint8_t bit) const {
    return (get(x, y, z) >> bit) & 1;
}

bool Map::getWall(int8_t x, int8_t y, int8_t z, uint8_t direc) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != ((direc + 2) % 4);
    else
        return (data >> (direc % 4)) & 1;
}

bool Map::getNorth(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 2;
    else
        return _GET_NORTH(data) & 1;
}

bool Map::getEast(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 3;
    else
        return _GET_EAST(data) & 1;
}

bool Map::getSouth(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 0;
    else
        return _GET_SOUTH(data) & 1;
}

bool Map::getWest(int8_t x, int8_t y, int8_t z) const {
    uint8_t data = get(x, y, z);
    if (_GET_TYPE(data) == RAMP_TILE)
        return _GET_DIR(data) != 1;
    else
        return _GET_WEST(data) & 1;
}

void Map::set(int8_t x, int8_t y, int8_t z, uint8_t data) {
    if (PanikFlags::getInstance().outOfRam()) {
        _freeAll();
        return;
    }

    // initial expansion when there is no data
    if (_layers == 0) {
        _data = (struct Layer*)malloc(sizeof(struct Layer));
        if (_data == NULL) {
            PanikFlags::getInstance().triggerOutOfRam();
            _freeAll();
            return;
        }
        _data[0] = {nullptr, 0, 0, 0, 0};
        _layers = 1;
        _offsetZ = -z;
    }

    int8_t rz = z + _offsetZ;

    // check and expand the x direction
    if (rz < 0) {
        uint8_t amount = abs(rz);
        struct Layer* newData = (struct Layer*)malloc(sizeof(struct Layer) * (_layers + amount));
        if (newData == NULL) {
            PanikFlags::getInstance().triggerOutOfRam();
            _freeAll();
            return;
        }
        memcpy(&newData[amount], _data, sizeof(struct Layer) * _layers);
        for (uint8_t i = 0; i < amount; i++) {
            newData[i] = {nullptr, 0, 0, 0, 0};
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
        if (newData == NULL) {
            PanikFlags::getInstance().triggerOutOfRam();
            _freeAll();
            return;
        }
        memcpy(newData, _data, sizeof(struct Layer) * _layers);
        for (uint8_t i = _layers; i < _layers + amount; i++) {
            newData[i] = {nullptr, 0, 0, 0, 0};
        }
        free(_data);
        _data = newData;
        _layers += amount;
    }

    struct Layer* currLayer = &_data[rz];

    // initial allocation when no data existing
    if (currLayer->width == 0 || currLayer->height == 0 || currLayer->data == nullptr) {
        currLayer->data = (uint8_t*)malloc(1);
        if (currLayer->data == NULL) {
            PanikFlags::getInstance().triggerOutOfRam();
            _freeAll();
            return;
        }
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
        uint8_t* newData = (uint8_t*)malloc((size_t)newWidth * (size_t)newHeight);
        if (newData == NULL) {
            PanikFlags::getInstance().triggerOutOfRam();
            _freeAll();
            return;
        }

        // bottom padding
        for (uint8_t iy = 0; iy < oldDataOffsetY; iy++) {
            for (uint8_t ix = 0; ix < newWidth; ix++)
                newData[iy * newWidth + ix] = _defData;
        }    
        for (uint8_t iy = 0; iy < currLayer->height; iy++) {
            // data copying
            for (uint8_t ix = 0; ix < currLayer->width; ix++)
                newData[(iy + oldDataOffsetY) * newWidth + (ix + oldDataOffsetX)] = currLayer->data[iy * currLayer->width + ix];
            // left padding
            for (uint8_t ix = 0; ix < oldDataOffsetX; ix++)
                newData[(iy + oldDataOffsetY) * newWidth + ix] = _defData;
            // right padding
            for (uint8_t ix = currLayer->width + oldDataOffsetX; ix < newWidth; ix++)
                newData[(iy + oldDataOffsetY) * newWidth + ix] = _defData;
        }
        // top padding
        for (uint8_t iy = currLayer->height + oldDataOffsetY; iy < newHeight; iy++) {
            for (uint8_t ix = 0; ix < newWidth; ix++)
                newData[iy * newWidth + ix] = _defData;
        }

        free(currLayer->data);
        currLayer->data = newData;
        currLayer->width = newWidth;
        currLayer->height = newHeight;
        currLayer->offsetX += oldDataOffsetX;
        currLayer->offsetY += oldDataOffsetY;
        rx += oldDataOffsetX;
        ry += oldDataOffsetY;
    }

    // final expanded data writing
    currLayer->data[ry * currLayer->width + rx] = data;
}

void Map::setBit(int8_t x, int8_t y, int8_t z, uint8_t bit, bool state) {
    uint8_t data = get(x, y, z);
    if (state)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    set(x, y, z, data);
}

void Map::setType(int8_t x, int8_t y, int8_t z, uint8_t type) {
    uint8_t data = get(x, y, z) & 0b00011111;
    data |= type << TILE_TYPE_START_BIT;
    set(x, y, z, data);
}

void Map::setRampDir(int8_t x, int8_t y, int8_t z, uint8_t dir) {
    uint8_t data = get(x, y, z) & 0b11111100;
    data |= dir & 0b11;
    set(x, y, z, data);
}

void Map::print(int8_t x, int8_t y, int8_t z) const {
    #if DEBUG_MODE
    DB_PRINTLN((getNorth(x, y, z) ? F("###") : F("# #")));
    DB_PRINT((getWest(x, y, z) ? "# " : "  ")); DB_PRINTLN((getEast(x, y, z) ? "#" : " "));
    DB_PRINT((getSouth(x, y, z) ? F("###") : F("# #")));
    #endif
}

// --------------------------------------------------
// operator overloads
// --------------------------------------------------

Map& Map::operator=(Map&& other) {
    if (this != &other) {
        _freeAll();
        _data = other._data;
        _layers = other._layers;
        _offsetZ = other._offsetZ;
        _defData = other._defData;
        other._data = nullptr;
        other._layers = 0;
        other._offsetZ = 0;
    }
    return *this;
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

void Map::_freeAll() {
    if (_data == nullptr)
        return;
    for (uint8_t z = 0; z < _layers; z++)
        if (_data[z].data != nullptr)
            free(_data[z].data);
    free(_data);
    _data = nullptr;
    _layers = 0;
    _offsetZ = 0;
}