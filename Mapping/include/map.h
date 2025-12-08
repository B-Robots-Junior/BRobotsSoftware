#ifndef _MAP_H_
#define _MAP_H_

#include <Arduino.h>
#include <bitPtr.h>
#include <pos.h>
#include <debug.h>

#define DEFAULT_DEF_DATA 0b00010000

#define NORTH_WALL_BIT 0
#define EAST_WALL_BIT 1
#define SOUTH_WALL_BIT 2
#define WEST_WALL_BIT 3
#define ADJACENT_TILES_BIT 4

#define TILE_TYPE_START_BIT 5 // inclusive
#define TILE_TYPE_END_BIT 8 // exlusive

#define RAMP_DIR_START_BIT 0 // inclusive
#define RAMP_DIR_END_BIT 2 // exlusive

#define RAMP_UP_BIT 2

// Tile types:
#define UNDISCOVERED_TILE 0
#define NORMAL_TILE 1
#define BLUE_TILE 2
#define BLACK_TILE 3
#define RAMP_TILE 4
#define CHECKPOINT_TILE 5

// if the tile is a ramp tile the data is used slightly differntly,
// because the data of the direction that changes the layer is stored in the first two bits
// then the data if the layer change is up is stored in bit2 and bit3 is unused, but adjacent tiles and type
// stays the same so you can check what data to use 

// The tile data looks like this:
//         | bit0  | bit1 | bit2  | bit3 | bit4    | bit5 | bit6 | bit7 |
// ----------------------------------------------------------------------
// normal: | north | east | south | west | adjTile |        type        |
// ramp:   |  facing dir  |  up?  |  -   | adjTile |        type        |

#define _GET_NORTH(data) ((data >> NORTH_WALL_BIT) & 1)
#define _GET_EAST(data) ((data >> EAST_WALL_BIT) & 1)
#define _GET_SOUTH(data) ((data >> SOUTH_WALL_BIT) & 1)
#define _GET_WEST(data) ((data >> WEST_WALL_BIT) & 1)
#define _GET_ADJ(data) ((data >> ADJACENT_TILES_BIT) & 1)
#define _GET_TYPE(data) ((data >> TILE_TYPE_START_BIT) & 0b111)
#define _GET_DIR(data) ((data >> RAMP_DIR_START_BIT) & 0b11)
#define _GET_UP(data) ((data >> RAMP_UP_BIT) & 1)

// tile constructer
#define TC(n, e, s, w, u, type) ((uint8_t)((\
    ((n & 1) << NORTH_WALL_BIT) |\
    ((e & 1) << EAST_WALL_BIT) |\
    ((s & 1) << SOUTH_WALL_BIT) |\
    ((w & 1) << WEST_WALL_BIT) |\
    ((u & 1) << ADJACENT_TILES_BIT) |\
    (type << TILE_TYPE_START_BIT)\
    ) & 0xFF))

#define RTC(dir, up, u, type) ((uint8_t)((\
    ((dir & 0b11) << RAMP_DIR_START_BIT) |\
    ((up & 1) << RAMP_UP_BIT) |\
    ((u & 1) << ADJACENT_TILES_BIT) |\
    (type << TILE_TYPE_START_BIT)\
    ) & 0xFF))

typedef Pos3<int8_t> mapPos; //can only go from x: -128 to 128 and y: -128 to 128 if more is needed just change to int16_t

class TileCon {
public:
    TileCon(uint8_t data);
    TileCon(bool north = false, bool east = false, bool south = false, bool west = false, bool adjacentTiles = true, uint8_t type = 0, uint8_t rampDir = 0, bool up = 0)
        : north(north), east(east), south(south), west(west), adjacentTiles(adjacentTiles), type(type), rampDir(rampDir), up(up) {}

    bool north, east, south, west, adjacentTiles;
    uint8_t type;
    // ramp stuff:
    uint8_t rampDir;
    bool up;

    void print() const;
    void println() const;

    BitPtr<1> operator[](int n); // gets the n'th bit of the tile data
    operator uint8_t();
};

class Layer {
public:
    uint8_t* _data = nullptr;
    uint8_t _width = 0;
    uint8_t _height = 0;
    int8_t _offsetY = 0;
    int8_t _offsetX = 0;

public:
    Layer() {}
    Layer(const Layer& other);
    Layer(Layer&& other);
    ~Layer() {
        free(_data);
    }

    uint64_t dynamicMemorySize();
    void defSate(); //! brings the Layer into a defined state, SHOULD ONLY BE USED AFTER MALLOC OR SIMILAR
    uint8_t get(int8_t x, int8_t y, uint8_t defData);
    bool set(int8_t x, int8_t y, uint8_t data, uint8_t defData); // if set returns true it means that malloc has failed and we hab moast probably run out of memory

    Layer& operator=(const Layer& other);
    Layer& operator=(Layer&& other);
};

class Map {
public:
    Layer* _data = nullptr;
    uint8_t _layers = 0;
    int8_t  _offsetZ = 0;
    uint8_t _defData;

public:
    Map(uint8_t defData = DEFAULT_DEF_DATA) : _defData(defData) {}
    Map(const Map& other);
    Map(Map&& other);
    ~Map() {
        for (uint8_t z = 0; z < _layers; z++)
            _data[z].~Layer();
        free(_data);
    }

    uint64_t dynamicMemorySize();
    uint8_t get(int8_t x, int8_t y, int8_t z) const;
    uint8_t get(mapPos pos) const {return get(pos.x, pos.y, pos.z);}
    bool getBit(int8_t x, int8_t y, int8_t z, uint8_t bit) const;
    bool getBit(mapPos pos, uint8_t bit) const {return getBit(pos.x, pos.y, pos.z, bit);}
    bool getWall(int8_t x, int8_t y, int8_t z, uint8_t direc) const;
    bool getWall(mapPos pos, uint8_t direc) const {return getWall(pos.x, pos.y, pos.z, direc);}
    bool getNorth(int8_t x, int8_t y, int8_t z) const;
    bool getNorth(mapPos pos) const {return getNorth(pos.x, pos.y, pos.z);}
    bool getEast(int8_t x, int8_t y, int8_t z) const;
    bool getEast(mapPos pos) const {return getEast(pos.x, pos.y, pos.z);}
    bool getSouth(int8_t x, int8_t y, int8_t z) const;
    bool getSouth(mapPos pos) const {return getSouth(pos.x, pos.y, pos.z);}
    bool getWest(int8_t x, int8_t y, int8_t z) const;
    bool getWest(mapPos pos) const {return getWest(pos.x, pos.y, pos.z);}
    bool getAdj(int8_t x, int8_t y, int8_t z) const {return getBit(x, y, z, ADJACENT_TILES_BIT);}
    bool getAdj(mapPos pos) const {return getAdj(pos.x, pos.y, pos.z);}
    bool getUp(int8_t x, int8_t y, int8_t z) const {return getBit(x, y, z, RAMP_UP_BIT);}
    bool getUp(mapPos pos) const {return getUp(pos.x, pos.y, pos.z);}
    uint8_t getType(int8_t x, int8_t y, int8_t z) const {return (get(x, y, z) >> TILE_TYPE_START_BIT) & 0b111;}
    uint8_t getType(mapPos pos) const {return getType(pos.x, pos.y, pos.z);}
    uint8_t getRampDir(int8_t x, int8_t y, int8_t z) const {return get(x, y, z) & 0b11;}
    uint8_t getRampDir(mapPos pos) const {return getRampDir(pos.x, pos.y, pos.z);}

    bool set(int8_t x, int8_t y, int8_t z, uint8_t data);
    bool set(mapPos pos, uint8_t data) {return set(pos.x, pos.y, pos.z, data);}
    bool setBit(int8_t x, int8_t y, int8_t z, uint8_t bit, bool state);
    bool setBit(mapPos pos, uint8_t bit, bool state) {return setBit(pos.x, pos.y, pos.z, bit, state);}
    bool setNorth(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, NORTH_WALL_BIT, state);}
    bool setNorth(mapPos pos, bool state) {return setNorth(pos.x, pos.y, pos.z, state);}
    bool setEast(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, EAST_WALL_BIT, state);}
    bool setEast(mapPos pos, bool state) {return setEast(pos.x, pos.y, pos.z, state);}
    bool setSouth(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, SOUTH_WALL_BIT, state);}
    bool setSouth(mapPos pos, bool state) {return setSouth(pos.x, pos.y, pos.z, state);}
    bool setWest(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, WEST_WALL_BIT, state);}
    bool setWest(mapPos pos, bool state) {return setWest(pos.x, pos.y, pos.z, state);}
    bool setAdj(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, ADJACENT_TILES_BIT, state);}
    bool setAdj(mapPos pos, bool state) {return setAdj(pos.x, pos.y, pos.z, state);}
    bool setUp(int8_t x, int8_t y, int8_t z, bool state) {return setBit(x, y, z, RAMP_UP_BIT, state);}
    bool setUp(mapPos pos, bool state) {return setUp(pos.x, pos.y, pos.z, state);}
    bool setType(int8_t x, int8_t y, int8_t z, uint8_t type);
    bool setType(mapPos pos, uint8_t type) {return setType(pos.x, pos.y, pos.z, type);}
    bool setRampDir(int8_t x, int8_t y, int8_t z, uint8_t dir);
    bool setRampDir(mapPos pos, uint8_t dir) {return setRampDir(pos.x, pos.y, pos.z, dir);}

    void print(int8_t x, int8_t y, int8_t z) const;
    void print(mapPos pos) const {print(pos.x, pos.y, pos.z);}
    void println(int8_t x, int8_t y, int8_t z) const {print(x, y, z); DB_PRINTLN();}
    void println(mapPos pos) const {println(pos.x, pos.y, pos.z);}

    Map& operator=(const Map& other);
    Map& operator=(Map&& other);
};

#endif