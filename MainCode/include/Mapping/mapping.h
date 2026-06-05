#ifndef MAPPING_H
#define MAPPING_H

#include <Mapping/map.h>
#include <pos.h>
#include <smartArray.h>

#define PANIC_RETURN_DEFAULT(default_val) do { if (_panic) return (default_val); } while (0)
#define PANIC_RETURN_VOID() do { if (_panic) return; } while (0)

extern const uint8_t tileWeights[6];
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

    bool operator==(const Move& other) { return rotation == other.rotation && distance == other.distance; }
    bool operator!=(const Move& other) { return rotation != other.rotation || distance != other.distance; }
};

class Mapper {
public: //! temporary for testing !SET THIS TO PRIVATE!
    volatile bool _panic = false;

    FastArray<mapPos> _actions;

    mapPos _targetPosition = mapPos(0, 0, 0);
    bool _gotTarget = true;
    Move _currentMove = Move(0, 0);
    bool _gotMove = false;
    uint8_t _currMoveMode = 0; // 0 is normal, 1 is returning to start, 2 is finished
    bool _inited = false;

    Move _panicMove = Move(0, 0);

    uint16_t _lastCheckpointActionIndex = 0; // action index of the last checkpoint driven over
    mapPos _lastCheckpointPos = mapPos(0, 0, 0);

    class NeighbourIterator {
    public:
        NeighbourIterator(uint8_t data, mapPos pos)
            : _rotation(4), _data(data), _pos(pos) {}

        bool next() {
            if (_GET_TYPE(_data) == RAMP_TILE) {
                if (_rotation == 4) {
                    _rotation = 0;
                    return true;
                }
                if (_rotation == 0) {
                    _rotation = 1;
                    return true;
                }
                _rotation = 4;
                return false;
            }

            if (_rotation == 4)
                _rotation = 0;
            else
                ++_rotation;

            while (_rotation < 4 && ((_data >> _rotation) & 1))
                ++_rotation;

            if (_rotation >= 4) {
                _rotation = 4;
                return false;
            }
            return true;
        }

        mapPos get() const {
            if (_rotation >= 4)
                return mapPos(0, 0, 0);

            if (_GET_TYPE(_data) == RAMP_TILE) {
                if (_rotation == 0)
                    return _pos + direcToVec[_GET_DIR(_data)] + mapPos(0, 0, _GET_UP(_data) ? 1 : -1);
                return _pos + direcToVec[(_GET_DIR(_data) + 2) % 4];
            }

            return _pos + direcToVec[_rotation];
        }

        uint8_t getDirec() const {
            if (_rotation >= 4)
                return 4;

            if (_GET_TYPE(_data) == RAMP_TILE)
                return (_rotation == 0) ? _GET_DIR(_data) : ((_GET_DIR(_data) + 2) % 4);

            return _rotation;
        }

        bool isEnd() const { return _rotation >= 4; }

        NeighbourIterator& operator++() { next(); return *this; }
        mapPos operator*() const { return get(); }

    private:
        uint8_t _rotation;
        uint8_t _data;
        mapPos _pos;
    };

    void _updateAdjTiles(mapPos updPos);
    mapPos _getLowerRampPos(mapPos checkPos);
    uint8_t _getUndiscoveredTileDeltaRot(mapPos currPos, uint8_t currRot);

public:
    Mapper() {
        _actions.push_back(mapPos(0, 0, 0));
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

    SimpleArray<mapPos> allreadySeenVictims; // stores all the positions of all of the allready seen victims so its not activated twice

    // actually needed functions from the outside:
    // --------------------------------------------------
    //! when the robot is reset the starting rotation (north 0) is assumed, but if the walls do not mach the given direction panicMode is forced
    void resetToLastCheckpoint(bool fWall, bool rWall, bool lWall, bool bWall);
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

    mapPos getNextTarget(); // returns 0, 0, 0 if the entire maze was discovered
    Move nextBestMoveTo(mapPos endPos);

    mapPos _getDriveInDirec(mapPos currPos, uint8_t currDirec); // this does not consider walls, but does consider ramps

    Mapper& operator=(const Mapper& other) = delete;
    Mapper& operator=(Mapper&& other) = delete;
};

#endif // MAPPING_H