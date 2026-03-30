#ifndef MAPPING_H
#define MAPPING_H

#include <Mapping/map.h>
#include <pos.h>
#include <smartArray.h>

#define PANIC_RETURN_DEFAULT(default_val) do { if (_panic) return (default_val); } while (0)
#define PANIC_RETURN_VOID() do { if (_panic) return; } while (0)

extern const mapPos direcToVec[4];
uint8_t vecToDirec(mapPos vec);
extern const uint8_t direcCheckOrder[4]; // the order in which the next tile to drive on is checked
extern const uint8_t direcCheckPriority[4]; // index is the direction and the value is the priority (exactly the reverse of direcCheckOrder)

int8_t deltaRotation(uint8_t start, uint8_t end); // calculates the amount of right rotations reqired to get to a rotation from a starting rotation

class Move {
public:
    Move() {}
    Move(int8_t rotation, uint8_t distance) : rotation(rotation), distance(distance) {}

    int8_t rotation = 0;
    uint8_t distance = 0; // should not drive backwards so its a uint8_t

    void print() const;
    void println() const;
};

class Action {
public:
    Action() {}
    Action(mapPos pos, uint16_t stepsAway) : pos(pos), stepsAway(stepsAway) {}

    mapPos pos = mapPos(0, 0, 0);
    uint16_t stepsAway = 0; // the number of steps away from 0, 0, 0 (start pos)
};

class Mapper {
private:
    volatile bool _panic = false;

    FastArray<Action> _actions;

    SimpleArray<Move> _currMoves = SimpleArray<Move>(Move(0, 0));
    uint16_t _currMoveIndex = 1;
    Move _panicMove = Move(0, 0);
    uint8_t _currMoveMode = 0; // 0 is normal, 1 is returning to start, 2 is finished

    uint16_t _lastCheckpointActionIndex = 0; // action index of the last checkpoint driven over

    void _updateAdjTiles(mapPos updPos);

    SimpleArray<mapPos> _getNeighbours(mapPos tilePos);

    SimpleArray<uint16_t> _getAllStepsAway(mapPos pos);
    Move _nextBestMoveTo(const SimpleArray<uint16_t>& endPosStepsAway, mapPos pos, uint8_t currRotation);
    uint16_t _minDistance(const SimpleArray<uint16_t>& stepsAwayA, const SimpleArray<uint16_t>& stepsAwayB);

    uint8_t _getUndiscAdjTileDirec(mapPos checkPos, uint8_t currentDirec);
    mapPos _getLowerRampPos(mapPos checkPos);

public:
    Mapper() {
        _actions.push_back(Action(mapPos(0, 0, 0), 0));
    }
    Mapper(const Mapper& other) = delete;
    Mapper(Mapper&& other) = delete;

    Map map;
    // 0, 0, 0 is the start pos
    // an increase of y is the south direction (2)
    // an increase of x is the east direction (1)
    // indexing can be negative, it will just increase the size of the map if not indexed before

    mapPos pos = mapPos(0, 0, 0);
    uint8_t rotation = 0; // rotation of the robot 0 is north 1 is east, 2 is south, 3 is west

    uint16_t stepsAwayFromStart = 0; // current steps away from start

    SimpleArray<mapPos> allreadySeenVictims; // stores all the positions of all of the allready seen victims so its not activated twice

    // actually needed functions from the outside:
    // --------------------------------------------------
    //! when the robot is reset the starting rotation (north 0) is assumed
    void resetToLastCheckpoint();
    // the bools are the walls (from tof measurement) in the current facing direction (used for panic mode)
    Move currMove(bool fWall, bool rWall, bool lWall, bool bWall, bool up, uint8_t type); // returns Move(0, 0) if the entire maze is discovered
    // if the current move leads to a black tile trigger this function instead of completeCurrMove() drive back and turn around
    void currMoveBlackTile(bool fWall, bool rWall, bool lWall, bool bWall);
    void completeCurrMove();

    bool hasAllreadySeenVictim(mapPos pos);
    void addSeenVictim(mapPos pos);

    //!!THIS IS NOT REVERSABLE!!
    void panicMode(); // enters panic mode (disreguards any previous mapping)
    bool isInPanicMode() const {return _panic;}
    // --------------------------------------------------

    uint64_t currentDynamicRamUsage(); // in bytes
    uint64_t currentRamUsage(); // in bytes

    void discover(TileCon tile, mapPos disPos);
    void discover(bool front, bool left, bool right, bool back, bool up, uint8_t type);
    void discover(bool front, bool left, bool right, bool back, bool up, uint8_t type, mapPos disPos, uint8_t disRotation);

    void turn(int8_t amount);
    void drive(bool forward = true);
    void driveAmount(int8_t amount);
    void driveMove(Move move);
    void driveMoves(const SimpleArray<Move>& moves);

    uint8_t goTo(mapPos endPos, mapPos startPos, SimpleArray<Move>* array, uint8_t startRotation); // works the same as the goTo below
    uint8_t goTo(mapPos endPos, SimpleArray<Move>* array); // array is the array it should append the movement and the return is the ending direction
    SimpleArray<Move> getNextMove(); // is it returns an empty array the entire maze was discovered

    mapPos _getDriveInDirec(mapPos currPos, uint8_t currDirec); // this does not consider walls, but does consider ramps

    Mapper& operator=(const Mapper& other) = delete;
    Mapper& operator=(Mapper&& other) = delete;
};

#endif // MAPPING_H