#include <map.h>
#include <debug.h>
#include <mapperDebugConsts.h>
#include <new>

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
// Layer class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// constructers
// --------------------------------------------------

Layer::Layer(const Layer& other) : _width(other._width), _height(other._height), _offsetX(other._offsetX), _offsetY(other._offsetY) {
    _data = (uint8_t*)malloc(_width * _height * sizeof(uint8_t));
    memcpy(_data, other._data, _width * _height * sizeof(uint8_t));
}

Layer::Layer(Layer&& other)
    : _width(other._width), _height(other._height), _offsetX(other._offsetX), _offsetY(other._offsetY), _data(other._data) {
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
    other._offsetX = 0;
    other._offsetY = 0;
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

uint64_t Layer::dynamicMemorySize() {
    return (_width * _height) * sizeof(uint8_t);
}

void Layer::defSate() {
    _data = nullptr;
    _width = 0;
    _height = 0;
    _offsetX = 0;
    _offsetY = 0;
}

uint8_t Layer::get(int8_t x, int8_t y, uint8_t defData) {
    int8_t rx = x + _offsetX;
    int8_t ry = y + _offsetY;
    if (rx < 0 || rx >= _width || ry < 0 || ry >= _height)
        return defData;
    return _data[ry * _width + rx];
}

bool Layer::set(int8_t x, int8_t y, uint8_t data, uint8_t defData) {
    if (_height == 0 || _width == 0) {
        _data = (uint8_t*)malloc(sizeof(uint8_t));
        if (_data == NULL) {
            _data = nullptr;
            return true;
        }
        _data[0] = data;
        _offsetX = -x; _offsetY = -y;
        _width = 1; _height = 1;
        return false;
    }

    int8_t rx = x + _offsetX;
    int8_t ry = y + _offsetY;
    if (rx >= 0 && rx < _width && ry >= 0 && ry < _height) {
        _data[ry * _width + rx] = data;
        return false;
    }
    
    uint8_t newHeight = _height;
    uint8_t newWidth = _width;
    int8_t newOffsetX = _offsetX;
    int8_t newOffsetY = _offsetY;

    if (ry < 0) {
        uint8_t amount = abs(ry);
        newHeight = _height + amount;
        newOffsetY = _offsetY + amount;
        ry += amount;
    }
    else if (ry >= _height) {
        newHeight = ry + 1;
    }

    if (rx < 0) {
        uint8_t amount = abs(rx);
        newWidth = _width + amount;
        newOffsetX += amount;
        rx += amount;
    }
    else if (rx >= _width) {
        newWidth = rx + 1;
    }

    uint8_t* newData = (uint8_t*)malloc(newHeight * newWidth * sizeof(uint8_t));
    if (newData == NULL) {
        newData = nullptr;
        return true;
    }
    memset(newData, defData, newHeight * newWidth * sizeof(uint8_t));
    for (uint8_t y = 0; y < _height; y++) {
        memcpy(&newData[(y + newOffsetY - _offsetY) * newWidth + (newOffsetX - _offsetX)], &_data[y * _width], _width * sizeof(uint8_t));
    }

    free(_data);
    _data = newData;
    _height = newHeight;
    _width = newWidth;
    _offsetX = newOffsetX;
    _offsetY = newOffsetY;

    _data[ry * _width + rx] = data;
    return false;
}

// --------------------------------------------------
// operator overloads
// --------------------------------------------------

Layer& Layer::operator=(const Layer& other) {
    if (&other == this)
        return *this;
    if (_width != other._width || _height != other._height) {
        free(_data);
        _width = other._width;
        _height = other._height;
        _data = (uint8_t*)malloc(_width * _height * sizeof(uint8_t));
    }
    _offsetX = other._offsetX;
    _offsetY = other._offsetY;
    memcpy(_data, other._data, _width * _height * sizeof(uint8_t));
    return *this;
}

Layer& Layer::operator=(Layer&& other) {
    if (&other == this)
        return *this;
    _data = other._data;
    _width = other._width;
    _height = other._height;
    _offsetX = other._offsetX;
    _offsetY = other._offsetY;
    // make the other one empty to ensure a memory safe state (in case the destructor gets called)
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
    other._offsetX = 0;
    other._offsetY = 0;
    return *this;
}

// ----------------------------------------------------------------------------------------------------
// Map class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// constructers
// --------------------------------------------------

Map::Map(const Map& other) : _layers(other._layers), _offsetZ(other._offsetZ), _defData(other._defData) {
    _data = (Layer*)malloc(_layers * sizeof(Layer));
    for (int i = 0; i < _layers; i++) {
        _data[i].defSate();
        _data[i] = other._data[i];
    }
}

Map::Map(Map&& other)
    : _data(other._data), _layers(other._layers), _offsetZ(other._offsetZ), _defData(other._defData) {
    other._data = nullptr;
    other._layers = 0;
    other._offsetZ = 0;
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

uint64_t Map::dynamicMemorySize() {
    uint64_t sum = _layers * sizeof(Layer);
    for (uint8_t z = 0; z < _layers; z++)
        sum += _data[z].dynamicMemorySize();
    return sum;
}

uint8_t Map::get(int8_t x, int8_t y, int8_t z) const {
    int8_t rz = z + _offsetZ;
    if (rz < 0 || rz >= _layers)
        return _defData;
    return _data[rz].get(x, y, _defData);
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

bool Map::set(int8_t x, int8_t y, int8_t z, uint8_t data) {
    if (_layers == 0) {
        _data = (Layer*)malloc(sizeof(Layer));
        if (_data == NULL) {
            _data = nullptr;
            return true;
        }
        _data[0].defSate();
        _data[0] = Layer();
        _offsetZ = -z;
        _layers = 1;
    }

    int8_t rz = z + _offsetZ;
    
    // resize layers
    if (rz < 0) {
        uint8_t amount = abs(rz);
        Layer* newData = (Layer*)malloc((_layers + amount) * sizeof(Layer));
        if (newData == NULL)
            return true;
        memcpy(&newData[amount], _data, _layers * sizeof(Layer));
        for (uint8_t i = 0; i < amount; i++)
            newData[i].defSate();
        _offsetZ += amount;
        _layers += amount;
        rz += amount;
        free(_data);
        _data = newData;
    }
    if (rz >= _layers) {
        uint8_t amount = rz - _layers + 1;
        Layer* newData = (Layer*)malloc((_layers + amount) * sizeof(Layer));
        if (newData == NULL)
            return true;
        memcpy(newData, _data, _layers * sizeof(Layer));
        for (uint8_t i = _layers; i < _layers + amount; i++)
            newData[i].defSate();
        free(_data);
        _data = newData;
        _layers += amount;
    }

    return _data[rz].set(x, y, data, _defData);
}

bool Map::setBit(int8_t x, int8_t y, int8_t z, uint8_t bit, bool state) {
    uint8_t data = get(x, y, z);
    if (state)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    return set(x, y, z, data);
}

bool Map::setType(int8_t x, int8_t y, int8_t z, uint8_t type) {
    uint8_t data = get(x, y, z) & 0b00011111;
    data |= type << TILE_TYPE_START_BIT;
    return set(x, y, z, data);
}

bool Map::setRampDir(int8_t x, int8_t y, int8_t z, uint8_t dir) {
    uint8_t data = get(x, y, z) & 0b11111100;
    data |= dir & 0b11;
    return set(x, y, z, data);
}

void Map::print(int8_t x, int8_t y, int8_t z) const {
    #if DEBUG_MODE
    String empty = type_atering[getType(x, y, z)] + " " + RESET_COLOR;
    DB_PRINTLN((getNorth(x, y, z) ? "###" : "#" + empty + "#"));
    DB_PRINT((getWest(x, y, z) ? "#" + empty : empty + empty)); DB_PRINTLN((getEast(x, y, z) ? "#" : empty));
    DB_PRINT((getSouth(x, y, z) ? "###" : "#" + empty + "#"));
    #endif
}

// --------------------------------------------------
// operator overloads
// --------------------------------------------------

Map& Map::operator=(const Map& other) {
    if (this == &other)
        return *this;
    if (_layers != other._layers) {
        for (uint8_t z = 0; z < _layers; z++)
            _data[z].~Layer();
        free(_data);
        _layers = other._layers;
        _data = (Layer*)malloc(_layers * sizeof(Layer));
        for (int i = 0; i < _layers; i++)
            _data[i].defSate();
    }
    _offsetZ = other._offsetZ;
    _defData = other._defData;
    for (int i = 0; i < _layers; i++)
        _data[i] = other._data[i];
    return *this;
}

Map& Map::operator=(Map&& other) {
    if (this == &other)
        return *this;
    _data = other._data;
    _layers = other._layers;
    _offsetZ = other._offsetZ;
    _defData = other._defData;
    other._data = nullptr;
    other._layers = 0;
    other._offsetZ = 0;
    return *this;
}
