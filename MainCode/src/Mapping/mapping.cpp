#include <Mapping/mapping.h>
#include <utility.h>
#include <TC/tc.h>

const uint8_t tileWeights[6] = {
    0, // undiscovered tile (should not come up)
    1, // normal tile
    100, // blue tile (avoid if possible)
    0, // black tile (should not come up)
    1, // ramp
    1 // checkpoint
};
const mapPos direcToVec[4] = {mapPos(0, -1, 0), mapPos(1, 0, 0), mapPos(0, 1, 0), mapPos(-1, 0, 0)};
const uint8_t direcCheckOrder[4] = {0, 3, 1, 2}; // the order in which the next tile to drive on is checked
const uint8_t direcCheckPriority[4] = {0, 2, 3, 1}; // index is the direction and the value is the priority (exactly the reverse of direcCheckOrder)

// ----------------------------------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------------------------------

uint8_t vecToDirec(mapPos vec) {
    vec.z = 0;
    for (uint8_t direc = 0; direc < 4; direc++)
        if (vec == direcToVec[direc])
            return direc;
    return 5;
}

int8_t deltaRotation(uint8_t start, uint8_t end) {
    int8_t delta = wrap(end - start, 0, 4);
    if (delta == 3)
        return -1;
    return delta;
}

// ----------------------------------------------------------------------------------------------------
// Move class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

void Move::print() const {
    DB_PRINT(F("Move("));
    DB_PRINT(rotation);
    DB_PRINT(", ");
    DB_PRINT(distance);
    DB_PRINT(")");
}

void Move::println() const {
    print();
    DB_PRINTLN();
}

// ----------------------------------------------------------------------------------------------------
// Mapper class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

// actually used methods from outside:
// --------------------------------------------------
void Mapper::resetToLastCheckpoint(bool fWall, bool rWall, bool lWall, bool bWall) {
    mapPos resetTile = pos;
    if (_gotMove && !_panic)
        resetTile = _getDriveInDirec(resetTile, wrap(rotation + _currentMove.rotation, 0, 4));
    if (_panicMove != Move(0, 0) && _panic)
        resetTile = _getDriveInDirec(resetTile, wrap(rotation + _panicMove.rotation, 0, 4));
    pos = _lastCheckpointPos;
    rotation = 0;
    PANIC_RETURN_VOID();

    map.setType(resetTile, BLACK_TILE);

    VAR_PRINTLN(_lastCheckpointActionIndex);

    // I know this is a very slow algorithm to dediscover tiles, but it works and we have time
    for (uint16_t i = _lastCheckpointActionIndex + 1; i < _actions.size(); i++) {
        bool skip = false;
        for (uint16_t j = 0; j <= _lastCheckpointActionIndex; j++) {
            if (_actions[i] == _actions[j]) {
                skip = true;
                break;
            }
        }
        if (skip)
            continue;
        // TODO: dediscover tile here and update undiscoverd adj tiles bit
        DB_PRINT_MUL((F("Dediscovering (x: "))(_actions[i].x)(F(", y: "))(_actions[i].y)(F(", z: "))(_actions[i].z)(F(")\n")));
        uint8_t data = map.get(_actions[i]);
        if (_GET_TYPE(data) == RAMP_TILE) {
            mapPos upPos = _actions[i] + (_GET_UP(data) ? mapPos(0, 0, 1) : mapPos(0, 0, -1)); 
            map.setType(upPos, UNDISCOVERED_TILE);
            DB_PRINT_MUL((F("Dediscovering (x: "))(upPos.x)(F(", y: "))(upPos.y)(F(", z: "))(upPos.z)(F(")\n")));
            map.setType(_actions[i], UNDISCOVERED_TILE);
            // update adjacent tiles for neighbours
            _updateAdjTiles(_actions[i] + direcToVec[_GET_DIR(data)] + (_GET_UP(data) ? mapPos(0, 0, 1) : mapPos(0, 0, -1)));
            _updateAdjTiles(_actions[i] + direcToVec[(_GET_DIR(data) + 2) % 4]);
            PANIC_RETURN_VOID();
            continue;
        }
        if (_GET_TYPE(data) == BLACK_TILE) { // we don't have to undiscover black tiles, in fact its better if we don't
            continue;
        }
        map.setType(_actions[i], UNDISCOVERED_TILE);
        for (uint8_t dir = 0; dir < 4; dir++) {
            if ((data >> dir) & 1) // theres a wall here so we can skip
                continue;
            DB_PRINT_MUL((F("   updating tile in direc: "))(dir)(F(" at: ")));
            (_actions[i] + direcToVec[dir]).println();
            _updateAdjTiles(_actions[i] + direcToVec[dir]);
            PANIC_RETURN_VOID();
        }
    }

    pos = _actions[_lastCheckpointActionIndex];
    _actions.remove_after(_lastCheckpointActionIndex); // removes all actions after _lastCheckpointActionIndex
    rotation = 0; //! Assumption that when the robot resets it is turned in the starting direction

    // check if the rotation matches, if not go into panic mode (this is so we can force a panic mode by placing it incorrectly)
    uint8_t data = map.get(pos);
    if (_GET_TYPE(data) != CHECKPOINT_TILE) {
        panicMode();
        return;
    }
    if (fWall != _GET_NORTH(data)) { panicMode(); return; }
    if (rWall != _GET_EAST(data)) { panicMode(); return; }
    if (bWall != _GET_SOUTH(data)) { panicMode(); return; }
    if (lWall != _GET_WEST(data)) { panicMode(); return; }

    _targetPosition = mapPos(0, 0, 0);
    _gotTarget = false;
    _currentMove = Move(0, 0);
    _gotMove = false;
    _currMoveMode = 0;
}

Move Mapper::currMove(bool fWall, bool rWall, bool lWall, bool bWall, bool up, uint8_t type) {
    if (type == CHECKPOINT_TILE)
        _lastCheckpointPos = pos;

    if (_panic) { // when in panic mode switch to an allways go left algorithm
        if (_panicMove.rotation != 0 || _panicMove.distance != 0)
            return _panicMove;

        if (!lWall)
            _panicMove = Move(-1, 1);
        else if (!fWall)
            _panicMove = Move(0, 1);
        else if (!rWall)
            _panicMove = Move(1, 1);
        else if (!bWall)
            _panicMove = Move(1, 0); // only turn once and then when its called next again to allow cameras to check the wall
        else
            _panicMove = Move(0, 0);

        return _panicMove;
    }

    if (!_inited) {
        discover(fWall, lWall, rWall, bWall, up, type);
        _inited = true;
    }

    // just in case this function is called after the first Move(0, 0)
    if (_currMoveMode == 2)
        return Move(0, 0);

    if (!_gotMove) {
        LACK;
        if (pos == _targetPosition && _gotTarget) {
            // we reached the target and now we need to drive into the undiscovered tile
            LACK;
            if (_currMoveMode == 1) {
                LACK;
                _currMoveMode = 2;
                return Move(0, 0);
            }
            _gotTarget = false;
            _gotMove = true;
            _currentMove = Move(_getUndiscoveredTileDeltaRot(pos, rotation), 1);
            PANIC_RETURN_DEFAULT(currMove(fWall, rWall, lWall, bWall, up, type));
            // return so that it doesn't yet find a new target, but instead drives this move first
            return _currentMove;
        }
        if (!_gotTarget) {
            LACK;
            if (_currMoveMode == 1) {
                LACK;
                _currMoveMode = 2;
                return Move(0, 0);
            }

            discover(fWall, lWall, rWall, bWall, up, type);

            LACK;
            _targetPosition = getNextTarget();
            if (_targetPosition == pos) {
                LACK;
                _gotMove = true;
                _currentMove = Move(_getUndiscoveredTileDeltaRot(pos, rotation), 1);
                PANIC_RETURN_DEFAULT(currMove(fWall, rWall, lWall, bWall, up, type));
                return _currentMove;
            }
            VAR_FUNC_PRINTLN(_targetPosition);
            LACK;
            _gotTarget = true;
            if (_targetPosition == mapPos(0, 0, 0) && !map.getAdj(_targetPosition)) { // if 0, 0, 0 has adjacent tiles it shoul not be counted
                LACK;
                _currMoveMode = 1;
            }
            LACK;
        }

        _currentMove = nextBestMoveTo(_targetPosition);
        LACK;
        PANIC_RETURN_DEFAULT(currMove(fWall, rWall, lWall, bWall, up, type));
        _gotMove = true;
    }

    return _currentMove;
}


void Mapper::currMoveBlackTile(bool fWall, bool rWall, bool lWall, bool bWall) {
    if (_panic) {
        _panicMove = Move(0, 0);
        return;
    }
    if (!_gotMove) {
        ERROR_MINOR(F("Trying to complete a move that is not given!"), SET_RED);
        return;
    }
    driveMove(_currentMove);
    discover(fWall, lWall, rWall, bWall, false, BLACK_TILE);
    if (_panic) {
        _panicMove = Move(0, 0);
        return;
    }
    driveMove(Move(2, 1));
    _gotMove = false;
}

void Mapper::completeCurrMove() {
    if (_panic) {
        driveMove(_panicMove);
        _panicMove = Move(0, 0);
        return;
    }
    if (!_gotMove) {
        ERROR_MINOR(F("Trying to complete a move that is not given!"), SET_RED);
        return;
    }
    driveMove(_currentMove);
    _gotMove = false;
}

bool Mapper::hasAllreadySeenVictim(mapPos pos) {
    PANIC_RETURN_DEFAULT(false);
    for (uint16_t i = 0; i < allreadySeenVictims.size(); i++) {
        if (allreadySeenVictims[i] == pos)
            return true;
    }
    return false;
}

void Mapper::addSeenVictim(mapPos pos) {
    PANIC_RETURN_VOID();
    allreadySeenVictims.push_back(pos);
    if (PanikFlags::getInstance().outOfRam())
        panicMode();
}

void Mapper::panicMode() {
    PANIC_RETURN_VOID();
    _panic = true;
    map = Map(); // delete map
    _actions = FastArray<mapPos>(); // delete actions
    allreadySeenVictims = SimpleArray<mapPos>(); // delete tracked victims
    DB_COLOR_PRINTLN(F("MAPPER ENTERED PANIC MODE, ERROR OCCOURED!"), SET_RED);
}
// --------------------------------------------------

uint64_t Mapper::currentDynamicRamUsage() {
    PANIC_RETURN_DEFAULT(0);
    uint64_t sum = map.dynamicMemorySize();
    sum += allreadySeenVictims.size() * sizeof(mapPos);
    sum += _actions.dataSize() * sizeof(mapPos);
    return sum;
}

uint64_t Mapper::currentRamUsage() {
    return currentDynamicRamUsage() + sizeof(Mapper);
}

void Mapper::discover(TileCon tile, mapPos disPos) {
    PANIC_RETURN_VOID();

    if (tile.type == UNDISCOVERED_TILE)
        tile.type = NORMAL_TILE;
    map.set(disPos, tile);

    if (tile.type == RAMP_TILE) {
        //uint8_t rampDirection = map.getRampDir(disPos);
        bool up = map.getUp(disPos);
        mapPos otherPos = mapPos(disPos.x, disPos.y, disPos.z + (up ? 1 : -1));
        map.set(otherPos, RTC((tile.rampDir + 2) % 4, !tile.up, true, RAMP_TILE)); // setup opposite ramp on the other layer
        if (!up) { // if a top ramp tile is discovered then you know that at least one action is pointing to a wrong value
            for (uint16_t i = 0; i < _actions.size(); i++) // correct all action positions
                _actions[i] = _getLowerRampPos(_actions[i]);
        }
        pos = _getLowerRampPos(pos); // can't hurt
    }

    // update current tile
    _updateAdjTiles(disPos);
    PANIC_RETURN_VOID();
    // update for all adjacent tiles
    NeighbourIterator iter(map.get(disPos), disPos);
    PANIC_RETURN_VOID();
    while (iter.next()) {
        _updateAdjTiles(iter.get());
        PANIC_RETURN_VOID();
    }
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return;
    }
}

void Mapper::discover(bool front, bool left, bool right, bool back, bool up, uint8_t type) {
    PANIC_RETURN_VOID();

    discover(front, left, right, back, up, type, pos, rotation);
}

void Mapper::discover(bool front, bool left, bool right, bool back, bool up, uint8_t type, mapPos disPos, uint8_t disRotation) {
    PANIC_RETURN_VOID();

    // if type is a ramp type it is assumed that the rotation is the ramp direction        
    TileCon data(0, 0, 0, 0, 0, type, disRotation, up); // the ramp data will only be used if it is a ramp type
    if (type != RAMP_TILE) {
        data[disRotation] = front;
        data[(disRotation + 1) % 4] = right;
        data[(disRotation + 2) % 4] = back;
        data[(disRotation + 3) % 4] = left;
    }

    discover(data, disPos);
}

void Mapper::turn(int8_t amount) {
    rotation = wrap(rotation + amount, 0, 4);
}

void Mapper::drive(bool forward) {
    // this checkpoint case is covered here and not in discover, because pos and _actions should only be modified here and discover may be run anywhere
    // this is here incase that the current tile is discovered after driven onto which is not covered by the case below
    if (!_panic) {
        if (map.getType(pos) == CHECKPOINT_TILE && _actions[_actions.size() - 1] == pos) {
            _lastCheckpointActionIndex = _actions.size() - 1;
            VAR_PRINTLN(_lastCheckpointActionIndex);
        }
    }

    pos = _getDriveInDirec(pos, (rotation + !forward * 2) % 4); // if not forward it shoud take the opposite direction

    // this checks if where driving onto a checkpoint tile
    if (!_panic) {
        if (map.getType(pos) == CHECKPOINT_TILE) {
            _lastCheckpointActionIndex = _actions.size();
            VAR_PRINTLN(_lastCheckpointActionIndex);
        }
        _actions.push_back(pos);
    }

    if (PanikFlags::getInstance().outOfRam())
        panicMode();
}

void Mapper::driveAmount(int8_t amount) {
    for (uint8_t i = 0; i < abs(amount); i++) {
        drive(amount >= 0);
    }
}

void Mapper::driveMove(Move move) {
    turn(move.rotation);
    driveAmount((int8_t)move.distance);
}

void Mapper::driveMoves(const SimpleArray<Move>& moves) {
    for (uint16_t i = 0; i < moves.size(); i++) {
        driveMove(moves[i]);
    }
}

mapPos Mapper::getNextTarget() {
    PANIC_RETURN_DEFAULT(mapPos(0, 0, 0));

    DB_PRINTLN(F("getNextTarget function call:"));

    for (int32_t i = (int32_t)_actions.size() - 1; i >= 0; i--) {
        mapPos currAction = _actions[i];
        DB_PRINT_MUL((F("  Checking _actions["))(i)(F("] ("))(currAction.x)(F(", "))(currAction.y)(F(", "))(currAction.z)(F(")\n")));
        if (map.getAdj(currAction) == false)
            continue;
        if (map.getType(currAction) == BLACK_TILE)
            continue; // cannot go to a black tile
        return currAction;
    }
    return mapPos(0, 0, 0);
}

Move Mapper::nextBestMoveTo(mapPos endPos) {
    PANIC_RETURN_DEFAULT(Move(0, 0));
    if (endPos == pos)
        return Move(0, 0);

    DB_PRINTLN(F("nextBestMoveTo function call:"));

    DB_PRINT(F("  "));
    VAR_FUNC_PRINTLN(endPos);

    NeighbourIterator iter(map.get(pos), pos);

    uint16_t minScore = 0xFFFF;
    uint8_t minDir = 4;

    while (iter.next()) {
        mapPos next = _getLowerRampPos(iter.get()); // top ramp poses won't be in the action list
        if (map.getType(next) == BLACK_TILE) // if its black we can't drive on it ovoiusly (this can create an inf. loop)
            continue;
        DB_PRINT(F("  "));
        VAR_FUNC_PRINTLN(next);
        if (next == endPos) {
            minScore = 0;
            minDir = iter.getDirec();
            break;
        }
        for (uint16_t i = 0; i < _actions.size(); i++) {
            if (_actions[i] != next) 
                continue;
            DB_PRINT(F("    "));
            VAR_PRINTLN(i);
            bool reachedTop = false;
            uint16_t topScore = 0;
            bool reachedBottom = false;
            uint16_t bottomScore = 0;
            for (uint16_t o = 1; !reachedTop || !reachedBottom; o++) {
                uint16_t topIndex = i + o;
                int32_t bottomIndex = static_cast<int32_t>(i) - o;
                if (topIndex < _actions.size() && !reachedTop) {
                    if (_actions[topIndex] == endPos)
                        reachedTop = true;
                    else
                        topScore += tileWeights[map.getType(_actions[topIndex])];
                }
                if (bottomIndex >= 0 && !reachedBottom) {
                    if (_actions[bottomIndex] == endPos)
                        reachedBottom = true;
                    else
                        bottomScore += tileWeights[map.getType(_actions[bottomIndex])];
                }
                if (topIndex >= _actions.size() && bottomIndex < 0)
                    break;
            }
            DB_PRINT(F("    "));
            VAR_PRINTLN(reachedTop);
            DB_PRINT(F("    "));
            VAR_PRINTLN(topScore);
            DB_PRINT(F("    "));
            VAR_PRINTLN(reachedBottom);
            DB_PRINT(F("    "));
            VAR_PRINTLN(bottomScore);
            if (reachedTop && topScore < minScore) {
                minScore = topScore;
                minDir = iter.getDirec();
            }
            if (reachedBottom && bottomScore < minScore) {
                minScore = bottomScore;
                minDir = iter.getDirec();
            }
        }
    }

    if (minDir == 4) {
        ERROR_MINOR(F("Could not find the next best move!"), SET_RED);
        panicMode();
        return Move(0, 0);
    }

    return Move(deltaRotation(rotation, minDir), 1);
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

mapPos Mapper::_getDriveInDirec(mapPos currPos, uint8_t currDirec) {
    PANIC_RETURN_DEFAULT(currPos + direcToVec[currDirec]);
    mapPos nextTile = currPos + direcToVec[currDirec];
    currPos = _getLowerRampPos(currPos); // ensures if on a ramp its on the lower one
    if (map.getType(currPos) == RAMP_TILE)
        if (map.getRampDir(currPos) == currDirec) // if on a ramp and facing the right direction move up
            nextTile.z += 1;
    currPos = _getLowerRampPos(nextTile); // _getLowerRampPos just in case we went onto a high tile ramp
    return currPos;
}

void Mapper::_updateAdjTiles(mapPos updPos) {
    PANIC_RETURN_VOID();
    if (map.getType(updPos) == UNDISCOVERED_TILE) // no point in updating an undiscovered tile
        return;
    bool hasAdjUndiscoveredTiles = false;
    updPos = _getLowerRampPos(updPos);
    NeighbourIterator iter(map.get(updPos), updPos);
    while (iter.next()) {
        if (map.getType(iter.get()) != UNDISCOVERED_TILE)
            continue;
        hasAdjUndiscoveredTiles = true;
        break;
    }
    map.setAdj(updPos, hasAdjUndiscoveredTiles);
    if (map.getType(updPos) == RAMP_TILE) // update linked ramps to the same value
        map.setAdj(mapPos(updPos.x, updPos.y, updPos.z + (map.getUp(updPos) ? 1 : -1)), hasAdjUndiscoveredTiles);
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return;
    }
}

mapPos Mapper::_getLowerRampPos(mapPos checkPos) {
    PANIC_RETURN_DEFAULT(checkPos);
    uint8_t data = map.get(checkPos); // uses one get to be faster (just for lower level code)
    if (_GET_TYPE(data) != RAMP_TILE)
        return checkPos;
    if (_GET_UP(data)) // if the ramp goes up its the lower of the two
        return checkPos;
    else
        return mapPos(checkPos.x, checkPos.y, checkPos.z - 1);
}

uint8_t Mapper::_getUndiscoveredTileDeltaRot(mapPos currPos, uint8_t currRot) {
    PANIC_RETURN_DEFAULT(0);

    uint8_t bestDir = 4;
    uint8_t priority = 0xFF;
    
    NeighbourIterator iter(map.get(currPos), currPos);

    // DB_PRINTLN(F("_getUndiscoveredTileDeltaRot call:"));
    // DB_PRINT((F("  Args: ")));
    // VAR_FUNC_PRINT(currPos);
    // DB_PRINT(F(", "));
    // VAR_PRINTLN(currRot);

    while (iter.next()) {
        if (map.getType(iter.get()) != UNDISCOVERED_TILE)
            continue;
        uint8_t dir = iter.getDirec();
        uint8_t pri = direcCheckPriority[((dir - currRot) + 4) % 4];
        // DB_PRINT_MUL((F("  dir: "))(dir)(F(":\n    pri: "))(pri)(F("\n    pos: ("))(iter.get().x)(", ")(iter.get().y)(", ")(iter.get().z)(")\n"));
        if (pri < priority) {
            priority = pri;
            bestDir = dir;
        }
    }
    // DB_PRINT_MUL(F("  Result: ")(bestDir)('\n'));
    if (bestDir == 4) {
        ERROR_MINOR(F("Could not find undiscovered adjacent tile"), SET_RED);
        panicMode();
        return 0;
    }

    return deltaRotation(currRot, bestDir);
}