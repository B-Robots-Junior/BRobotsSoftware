#pragma once

#ifndef _MAPPING_H_
#define _MAPPING_H_

#include <array.h>
#include <map.h>
#include <pos.h>

#define MAP_LAYERS 2

static const mapPos direcToVec[4] = {mapPos(0, -1), mapPos(1, 0), mapPos(0, 1), mapPos(-1, 0)};
uint8_t vecToDirec(mapPos vec);
static const uint8_t direcCheckOrder[4] = {0, 3, 1, 2}; // the order in which the next tile to drive on is checked
static const uint8_t direcCheckPriority[4] = {0, 2, 3, 1}; // index is the direction and the value is the priority (exactly the reverse of direcCheckOrder)

// TODO: implement a way to handle danger zones! (should at least be able to switch beween pass through and avoid at all costs)
double wrap(double value, double lowerBound, double upperBound);
int8_t deltaRotation(uint8_t start, uint8_t end); // calculates the amount of right rotations reqired to get to a rotation from a starting rotation

class Move {
public:
    Move(int8_t rotation, uint8_t distance) : rotation(rotation), distance(distance) {}

    int8_t rotation;
    uint8_t distance; // should not drive backwards so its a uint8_t

    void print() const;
    void println() const;
};

class Action {
public:
    Action(mapPos pos, uint16_t stepsAway) : pos(pos), stepsAway(stepsAway) {}

    mapPos pos;
    uint16_t stepsAway; // the number of steps away from 0, 0, 0 (start pos)
};

class Mapper {
private:
    bool _panic = false;

    mapPos _getDriveInDirec(mapPos currPos, uint8_t currDirec); // this does not consider walls, but does consider ramps
    void _updateAdjTiles(mapPos updPos);

    Array<mapPos> _getNeighbours(mapPos tilePos);

    Array<uint16_t> _getAllStepsAway(mapPos pos);
    Move _nextBestMoveTo(const Array<uint16_t>& endPosStepsAway, mapPos pos, uint8_t currRotation);
    uint16_t _minDistance(const Array<uint16_t>& stepsAwayA, const Array<uint16_t>& stepsAwayB);

    uint8_t _recursiveGoTo(mapPos endPos, const Array<uint16_t>& endPosStepsAway, mapPos startPos, Array<Move>* array, uint8_t startRotation);

    uint8_t _getUndiscAdjTileDirec(mapPos checkPos, uint8_t currentDirec);
    mapPos _getLowerRampPos(mapPos checkPos);

public:
    Mapper() {}

    Map map;
    // 0, 0, 0 is the start pos
    // an increase of y is the south direction (2)
    // an increase of x is the east direction (1)
    // indexing can be negative, it will just increase the size of the map if not indexed before

    Array<Action> actions = Array<Action>({Action(mapPos(0, 0, 0), 0)}); // all the tiles, in order, that have been driven over (starting value {(0,0)}, so it can find back to start)
    mapPos pos = mapPos(0, 0, 0);
    uint8_t rotation = 0; // rotation of the robot 0 is north 1 is east, 2 is south, 3 is west

    uint16_t stepsAwayFromStart = 0; // current steps away from start
    uint16_t lastCheckpointStepsFromStart = 0; // when reset it should reset stepsAwayFromStart to this value and reset position
    mapPos lastCheckpointPosition = mapPos(0, 0);

    Array<mapPos> allreadySeenVictims; // stores all the positions of all of the allready seen victims so its not activated twice

    uint64_t currentDynamicRamUsage(); // in bytes
    uint64_t currentRamUsage(); // in bytes

    void discover(TileCon tile, mapPos disPos);
    void discover(bool front, bool left, bool right, bool back, bool up, uint8_t type);
    void discover(bool front, bool left, bool right, bool back, bool up, uint8_t type, mapPos disPos, uint8_t disRotation);

    void turn(int8_t amount);
    void drive(bool forward = true);
    void driveAmount(int8_t amount);
    void driveMove(Move move);
    void driveMoves(const Array<Move>& moves);

    //! when the robot is reset the starting rotation (north 0) is assumed
    void resetToLastCheckpoint();

    uint8_t goTo(mapPos endPos, mapPos startPos, Array<Move>* array, uint8_t startRotation); // works the same as the goTo below
    uint8_t goTo(mapPos endPos, Array<Move>* array); // array is the array it should append the movement and the return is the ending direction
    // the bools are the walls (from tof measurement) in the current facing direction (used for panic mode)
    Array<Move> getNextMove(bool fWall, bool rWall, bool lWall, bool bWall); // is it returns an empty array the entire maze was discovered

    bool hasAllreadySeenVictim(mapPos pos);
    void addSeenVictim(mapPos pos);

    //!!THIS IS NOT REVERSABLE!!
    void panicMode(); // enters panic mode (disreguards any previous mapping)
};

#endif