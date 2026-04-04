#include <mapping.h>
#include <utility.h>

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
    PANIC_RETURN_VOID();

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

    _currMoves = getNextMove();
    _currMoveIndex = 0;
}

Move Mapper::currMove(bool fWall, bool rWall, bool lWall, bool bWall, bool up, uint8_t type) {
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

    // just in case this function is called after the first Move(0, 0)
    if (_currMoveMode == 2)
        return Move(0, 0);

    if (_currMoveIndex >= _currMoves.size()) {
        // if state is 1 then the robot has found its way back
        if (_currMoveMode == 1) {
            _currMoveMode = 2;
            return Move(0, 0); // return this to indicate successfull completion of the maze
        }
        discover(fWall, lWall, rWall, bWall, up, type);
        _currMoves = getNextMove();
        PANIC_RETURN_DEFAULT(currMove(fWall, rWall, lWall, bWall, up, type));
        _currMoveIndex = 0;
    }

    // if getNextMove sais there is nothing left, and I need something go back to start
    if (_currMoves.size() == 0 && _currMoveMode == 0) {
        goTo(mapPos(0, 0, 0), &_currMoves); // get the way back to start
        PANIC_RETURN_DEFAULT(currMove(fWall, rWall, lWall, bWall, up, type));
        if (_currMoves.size() == 0)
            return Move(0, 0);
        _currMoveIndex = 0;
        _currMoveMode = 1; // set state to returning to start
    }
    
    return _currMoves[_currMoveIndex];
}


void Mapper::currMoveBlackTile(bool fWall, bool rWall, bool lWall, bool bWall) {
    PANIC_RETURN_VOID();
    driveMove(_currMoves[_currMoveIndex]);
    discover(fWall, lWall, rWall, bWall, false, BLACK_TILE);
    if (_panic)
        return;
    driveMove(Move(2, 1));
    _currMoves = getNextMove();
    _currMoveIndex = 0;
}

void Mapper::completeCurrMove() {
    if (_panic) {
        _panicMove = Move(0, 0);
        return;
    }
    if (_currMoveIndex >= _currMoves.size()) {
        ERROR_MINOR(F("Trying to complete a move that is not given!"), SET_RED);
        return;
    }
    driveMove(_currMoves[_currMoveIndex]);
    _currMoveIndex++;
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
        pos = _getLowerRampPos(pos); // can't hurt
    }

    // update current tile
    _updateAdjTiles(disPos);
    PANIC_RETURN_VOID();
    // update for all adjacent tiles
    SimpleArray<mapPos> updates = _getNeighbours(disPos);
    PANIC_RETURN_VOID();
    for (uint16_t i = 0; i < updates.size(); i++) {
        _updateAdjTiles(updates[i]);
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
    PANIC_RETURN_VOID();

    rotation = wrap(rotation + amount, 0, 4);
}

void Mapper::drive(bool forward) {
    PANIC_RETURN_VOID();

    // this checkpoint case is covered here and not in discover, because pos and _actions should only be modified here and discover may be run anywhere
    // this is here incase that the current tile is discovered after driven onto which is not covered by the case below
    if (map.getType(pos) == CHECKPOINT_TILE && _actions[_actions.size() - 1] == pos) {
        _lastCheckpointActionIndex = _actions.size() - 1;
        VAR_PRINTLN(_lastCheckpointActionIndex);
    }

    pos = _getDriveInDirec(pos, (rotation + !forward * 2) % 4); // if not forward it shoud take the opposite direction

    // this checks if where driving onto a checkpoint tile
    if (map.getType(pos) == CHECKPOINT_TILE) {
        _lastCheckpointActionIndex = _actions.size();
        VAR_PRINTLN(_lastCheckpointActionIndex);
    }
    _actions.push_back(pos);

    if (PanikFlags::getInstance().outOfRam())
        panicMode();
}

void Mapper::driveAmount(int8_t amount) {
    for (uint8_t i = 0; i < abs(amount); i++) {
        PANIC_RETURN_VOID();
        drive(amount >= 0);
    }
}

void Mapper::driveMove(Move move) {
    PANIC_RETURN_VOID();

    turn(move.rotation);
    driveAmount((int8_t)move.distance);
}

void Mapper::driveMoves(const SimpleArray<Move>& moves) {
    PANIC_RETURN_VOID();

    for (uint16_t i = 0; i < moves.size(); i++) {
        PANIC_RETURN_VOID();
        driveMove(moves[i]);
    }
}

uint8_t Mapper::goTo(mapPos endPos, mapPos startPos, SimpleArray<Move>* array, uint8_t startRotation) {
    PANIC_RETURN_DEFAULT(startRotation);

    mapPos currPos = startPos;
    uint8_t currRotation = startRotation;
    SimpleArray<uint16_t> endStepsAway = _getAllStepsAway(endPos);
    PANIC_RETURN_DEFAULT(startRotation);
    while (!(currPos == endPos))
    {
        Move thisMove = _nextBestMoveTo(endStepsAway, currPos, currRotation);
        PANIC_RETURN_DEFAULT(startRotation);
        if (thisMove.rotation == 0 && array->size() != 0)
            (*array)[array->size() - 1].distance++;
        else {
            Move splitMove = thisMove;
            if (abs(thisMove.rotation) == 2) {
                array->push_back(Move(1, 0));
                if (PanikFlags::getInstance().outOfRam()) {
                    panicMode();
                    return startRotation;
                }
                splitMove.rotation = 1;
            }
            array->push_back(splitMove);
            if (PanikFlags::getInstance().outOfRam()) {
                panicMode();
                return startRotation;
            }
        }
        currRotation = wrap(currRotation + thisMove.rotation, 0, 4);
        for (uint8_t move = 0; move < thisMove.distance; move++)
            currPos = _getDriveInDirec(currPos, currRotation);
    }
    return currRotation;
}

uint8_t Mapper::goTo(mapPos endPos, SimpleArray<Move>* array) {
    uint8_t rot = goTo(endPos, pos, array, rotation);
    PANIC_RETURN_DEFAULT(5);
    return rot;
}

SimpleArray<Move> Mapper::getNextMove() {
    PANIC_RETURN_DEFAULT(SimpleArray<Move>());
    DB_PRINT_MUL((SET_GREEN)(F("Started getNextMove\n"))(RESET_COLOR));
    
    // this is a bit of a nitpick, but it would be more efficient to get all occourences of the current tile your on
    // and then search the neighbouring indexes or to be a bit more efficient, but more comliated use a Breadth-First Search
    VAR_PRINTLN(_actions.size());
    for (int32_t i = (int32_t)_actions.size() - 1; i >= 0; i--) {
        mapPos currAction = _actions[i];
        DB_PRINT_MUL((F("Checking "))(i)(F(" ("))(currAction.x)(F(", "))(currAction.y)(F(", "))(currAction.z)(F(")\n")));
        if (map.getAdj(currAction) == false)
            continue;
        if (map.getType(currAction) == BLACK_TILE)
            continue; // cannot go to a black tile

        DB_PRINT_MUL((F("currAction: ("))(currAction.x)(", ")(currAction.y)(", ")(currAction.z)(")\n"));

        SimpleArray<Move> moves;
        currAction = _getLowerRampPos(currAction);
        uint8_t endRoation = goTo(currAction, &moves);
        VAR_PRINTLN(endRoation);
        VAR_PRINTLN(moves.size());
        PANIC_RETURN_DEFAULT(SimpleArray<Move>());

        uint8_t undiscoveredAdjTileDirec = _getUndiscAdjTileDirec(currAction, endRoation);
        VAR_PRINTLN(undiscoveredAdjTileDirec);
        if (_panic) {
            //ERROR("current tile has been marked as having adj. undisc. tiles, but turned out to have none check tile updating!");
            ERROR_MINOR(F("current tile has been marked as having adj. undisc. tiles, but turned out to have none check tile updating!"), SET_RED);
            return SimpleArray<Move>();
        }

        int8_t moveRotation = deltaRotation(endRoation, undiscoveredAdjTileDirec);
        VAR_PRINTLN(moveRotation);
        PANIC_RETURN_DEFAULT(SimpleArray<Move>());
        if (moveRotation == 0 && moves.size() != 0)
            moves[moves.size() - 1].distance++;
        else {
            if (moveRotation == 2) { // this case is to give time for the Camera to detect victims
                moves.push_back(Move(1, 0));
                moves.push_back(Move(1, 1));
                if (PanikFlags::getInstance().outOfRam()) {
                    panicMode();
                    return SimpleArray<Move>();
                }
            }
            else {
                moves.push_back(Move(moveRotation, 1));
                if (PanikFlags::getInstance().outOfRam()) {
                    panicMode();
                    return SimpleArray<Move>();
                }
            }
        }
        DB_PRINT_MUL((SET_GREEN)(F("End getNextMove\n"))(RESET_COLOR));
        return moves;
    }
    return SimpleArray<Move>();
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

// private methods are not edited for panic mode!

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
    SimpleArray<mapPos> neighbours = _getNeighbours(updPos);
    PANIC_RETURN_VOID();
    for (uint16_t i = 0; i < neighbours.size(); i++) {
        if (map.getType(neighbours[i]) != UNDISCOVERED_TILE)
            continue;
        hasAdjUndiscoveredTiles = true;
        break;
    }
    map.setAdj(updPos, hasAdjUndiscoveredTiles);
    if (map.getType(updPos) == RAMP_TILE) // update linked ramps to the same value
        map.setAdj(mapPos(updPos.x, updPos.y, updPos.z + 1), hasAdjUndiscoveredTiles);
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return;
    }
}

SimpleArray<mapPos> Mapper::_getNeighbours(mapPos tilePos) {
    PANIC_RETURN_DEFAULT(SimpleArray<mapPos>());
    SimpleArray<mapPos> neighbours;
    tilePos = _getLowerRampPos(tilePos); // ensure on lower ramp
    if (map.getType(tilePos) == RAMP_TILE) {
        uint8_t rampDirec = map.getRampDir(tilePos);
        mapPos upPos = tilePos + direcToVec[rampDirec];
        upPos.z += 1;
        neighbours.push_back(upPos);
        neighbours.push_back(tilePos + direcToVec[(rampDirec + 2) % 4]);
        if (PanikFlags::getInstance().outOfRam()) {
            panicMode();
            return SimpleArray<mapPos>();
        }
        return neighbours;
    }
    for (uint8_t direc = 0; direc < 4; direc++) {
        if (map.getWall(tilePos, direc))
            continue;
        neighbours.push_back(tilePos + direcToVec[direc]);
        if (PanikFlags::getInstance().outOfRam()) {
            panicMode();
            return SimpleArray<mapPos>();
        }
    }
    return neighbours;
}

SimpleArray<uint16_t> Mapper::_getAllStepsAway(mapPos pos) {
    PANIC_RETURN_DEFAULT(SimpleArray<uint16_t>());
    pos = _getLowerRampPos(pos);
    SimpleArray<uint16_t> stepsAway;
    for (uint16_t i = 0; i < _actions.size(); i++) {
        mapPos currAction = _actions[i];
        currAction = _getLowerRampPos(currAction); //! CHECK IF THIS DOESN'T CAUSE MASSIVE TIME LOSS
        _actions[i] = currAction;
        // because looking up the type and upadting it for each move is a bit accessive, but sometimes not really because its usefull
        if (currAction == pos)
            stepsAway.push_back(i);
    }
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return SimpleArray<uint16_t>();
    }
    return stepsAway;
}

Move Mapper::_nextBestMoveTo(const SimpleArray<uint16_t>& endPosStepsAway, mapPos currPos, uint8_t currRotation) {
    PANIC_RETURN_DEFAULT(Move(0, 0));
    uint16_t minDist = 0xFFFF;
    SimpleArray<mapPos> neighbours = _getNeighbours(currPos);
    PANIC_RETURN_DEFAULT(Move(0, 0));
    uint16_t minIndex = neighbours.size();
    for (uint16_t i = 0; i < neighbours.size(); i++) {
        if (map.getType(neighbours[i]) == BLACK_TILE)
            continue; // basically the same thing as a wall
        uint16_t currDist = _minDistance(endPosStepsAway, _getAllStepsAway(neighbours[i]));
        PANIC_RETURN_DEFAULT(Move(0, 0));
        if (currDist < minDist) {
            minDist = currDist;
            minIndex = i;
        }
    }
    if (minIndex == neighbours.size()) {
        //ERROR("min direction couldn't be found (should be impossible)!");
        ERROR_MINOR(F("min direction couldn't be found (should be impossible)!"), SET_RED);
        panicMode();
        return Move(0, 0);
    }
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return Move(0, 0);
    }
    // no need to think about ramps, because vecToDirec sets the z value to 0 automatically
    uint8_t dir = vecToDirec(neighbours[minIndex] - currPos);
    if (dir == 5) {
        ERROR_MINOR(F("vecToDirec returned invalid value 5!"), SET_RED);
        panicMode();
        return Move(0, 0);
    }
    return Move(deltaRotation(currRotation, dir), 1);
}

uint16_t Mapper::_minDistance(const SimpleArray<uint16_t>& stepsAwayA, const SimpleArray<uint16_t>& stepsAwayB) {
    PANIC_RETURN_DEFAULT(0xFFFF);
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return 0xFFFF;
    }
    uint16_t minDist = 0xFFFF;
    for (uint16_t i = 0; i < stepsAwayA.size(); i++) {
        for (uint16_t j = 0; j < stepsAwayB.size(); j++) {
            uint16_t dist = (stepsAwayA[i] > stepsAwayB[j]) ? 
                 (stepsAwayA[i] - stepsAwayB[j]) : 
                 (stepsAwayB[j] - stepsAwayA[i]);
            if (dist < minDist)
                minDist = dist;
        }
    }
    return minDist;
}

uint8_t Mapper::_getUndiscAdjTileDirec(mapPos checkPos, uint8_t currentDirec) {
    PANIC_RETURN_DEFAULT(5);
    checkPos = _getLowerRampPos(checkPos);
    SimpleArray<mapPos> neighbours = _getNeighbours(checkPos);
    PANIC_RETURN_DEFAULT(5);
    uint8_t leastPriority = 5;
    uint8_t bestDirec = 5;
    for (uint16_t i = 0; i < neighbours.size(); i++) {
        if (map.getType(neighbours[i]) != UNDISCOVERED_TILE)
            continue;
        uint8_t direc = vecToDirec(neighbours[i] - checkPos);
        if (direc == 5) {
            ERROR_MINOR(F("vecToDirec returned invalid value 5!"), SET_RED);
            panicMode();
            return 5;
        }
        uint8_t relDirec = wrap(direc - currentDirec, 0, 4);
        if (direcCheckPriority[relDirec] < leastPriority) {
            leastPriority = direcCheckPriority[relDirec];
            bestDirec = direc;
        }
    }
    if (bestDirec == 5) {
        //ERROR("This function was used on a tile with no adj uned tiles!");
        ERROR_MINOR(F("This function was used on a tile with no adj uned tiles!"), SET_RED);
        panicMode();
    }
    if (PanikFlags::getInstance().outOfRam()) {
        panicMode();
        return 5;
    }
    return bestDirec;
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