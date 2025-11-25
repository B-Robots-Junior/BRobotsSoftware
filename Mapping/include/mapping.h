#ifndef _MAPPING_H_
#define _MAPPING_H_

#include <Arduino.h>
#include <debug.h>
#include <mapE.h>
#include <pos.h>
#include <primArray.h>
#include <queue.h>

#define RISING_THREASHOLD 20

// global vars
// --------------------------------------------------

extern mapPos direcLookup[4];
extern uint8_t tileWeights[6]; // how fast we can cross each type of tile for the optimal path calculation

// --------------------------------------------------

class MapRotation {
private:
    uint8_t _rotation;

public: 
    MapRotation(uint8_t rotation = 0) : _rotation(rotation % 4) {}

    uint8_t getRotation() const {return _rotation;}
    void setRotation(uint8_t rotation) {_rotation = rotation % 4;}

    MapRotation operator+(MapRotation other) {
        return MapRotation((_rotation + other._rotation) % 4);
    }
    MapRotation operator+(int8_t other) {
        int8_t resRot = _rotation + other;
        while (resRot < 0)
            resRot += 4;
        return MapRotation(resRot % 4);
    }
    void operator+=(int8_t other) {
        _rotation = (*this + other).getRotation();
    }
    MapRotation operator-(MapRotation other) {
        int8_t rot = _rotation - other._rotation;
        if (rot < 0)
            rot += 4;
        return MapRotation(rot % 4);
    }
};

class Action {
public:
    Action(int8_t rotation = 0, uint8_t drive = 0) : rotation(rotation), drive(drive) {}

    int8_t rotation;
    uint8_t drive;

    void apply(mapPos& pos, MapRotation& rot, const MapE& map) {
        apply(pos, rot, map.get(pos));
    }
    void apply(mapPos& pos, MapRotation& rot, uint8_t tileData) {
        rot += rotation;
        if (_GET_TYPE(tileData) == RAMP_TILE) // this is added to make shure it goes up and down ramps correctly
            if (rot.getRotation() == _GET_DIR(tileData))
                pos.z += _GET_UP(tileData) ? 1 : -1;
        pos.x += direcLookup[rot.getRotation()].x * drive;
        pos.y += direcLookup[rot.getRotation()].y * drive;
    }
};

Likely<Action> posDiffToAction(MapRotation rot, mapPos diff);
int8_t diffRotation(MapRotation a, MapRotation b); // returns rotation needed to get from a to b

class Mapper {
private:
    mapPos _pos;
    MapRotation _rotation;

    bool _panicMode = false;

    void _updateAdjTilesBit(mapPos pos);
    void _updateAllAdjTiles(mapPos pos);

public:
    Mapper() {}

    // creates a map that may use the entire EEPROM
    //! this has to bechanged to account for the spectrometer and rgb sensor data and possibly refectiveness
    MapE map = MapE(); //! CAUTION READ ABOVE

    mapPos getPos() const {return _pos;}

    // discover the current position
    void discover(float risingAngle, bool front, bool back, bool left, bool right, uint8_t tileType);
    void discover(MapRotation rot, mapPos pos, float risingAngle, bool front, bool back, bool left, bool right, uint8_t tileType);
    Likely<PrimArray<Action>> goTo(mapPos origin, MapRotation rotation, mapPos end);
    uint8_t getNeighbors(mapPos pos, mapPos* neighbors); // a 4 element array has to be given as the neighbors argument and it will return the use lenght
    uint8_t getNeighbors(mapPos pos, uint8_t data, mapPos* neighbors); // a 4 element array has to be given as the neighbors argument and it will return the use lenght

    void panic() {_panicMode = true;} // call this when an error occours in the mapping. It will set mapping into a panic mode with a only drive left algorithm
};

#endif