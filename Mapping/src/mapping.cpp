#include <mapping.h>

// ----------------------------------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------------------------------

double wrap(double value, double lowerBound, double upperBound) {
    double rangeSize = upperBound - lowerBound;

    if (rangeSize <= 0) return lowerBound;
    double wrappedValue = fmod((value - lowerBound), rangeSize);
    if (wrappedValue < 0) {
        wrappedValue += rangeSize;
    }

    return wrappedValue + lowerBound;
}

uint8_t vecToDirec(mapPos vec) {
    vec.z = 0;
    for (int direc = 0; direc < 4; direc++)
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
    DB_PRINT("Move(");
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

uint64_t Mapper::currentDynamicRamUsage() {
    uint64_t sum = map.dynamicMemorySize();
    sum += actions.size() * sizeof(Action);
    sum += allreadySeenVictims.size() * sizeof(mapPos);
    return sum;
}

uint64_t Mapper::currentRamUsage() {
    return currentDynamicRamUsage() + sizeof(Mapper);
}

void Mapper::discover(TileCon tile, mapPos disPos) {
    if (_panic)
        return;
    
    map.set(disPos, tile);
    if (tile.type == UNDISCOVERED_TILE)
        map.setType(disPos, NORMAL_TILE);

    if (tile.type == RAMP_TILE) {
        //uint8_t rampDirection = map.getRampDir(disPos);
        bool up = map.getUp(disPos);
        mapPos otherPos = mapPos(disPos.x, disPos.y, disPos.z + (up ? 1 : -1));
        map.set(otherPos, RTC((tile.rampDir + 2) % 4, !tile.up, true, RAMP_TILE)); // setup opposite ramp on the other layer
        //map[MAP_LAYERS - disPos.z - 1][disPos.y][disPos.x] = TileCon(1, 1, 1, 1, true, RAMP_TILE);
        //map[MAP_LAYERS - disPos.z - 1][disPos.y][disPos.x][rampDirection] = false; // set ramp direction wall on the other layer ramp false
        
        // all ramp updates are with getNeighbours, so this is no longer needed
        //_updateAdjTiles(mapPos(disPos.x, disPos.y, MAP_LAYERS - disPos.z - 1));
        //// update all top or bottom adj Tiles
        //for (int direc = 0; direc < 4; direc++) {
        //    mapPos dirPos = disPos + direcToVec[direc];
        //    dirPos.z = MAP_LAYERS - dirPos.z - 1; // gets the other layer
        //    _updateAdjTiles(dirPos);
        //}

        pos = _getLowerRampPos(pos); // can't hurt
        //VAR_PRINTLN(disPos == pos || otherPos == pos)
        //if (disPos == pos || otherPos == pos) { // if you are on the same tile
        //    if (tile.up == true) // check if the current layer is the bottom one of the ramp and set it to the bottom one
        //        pos.z = disPos.z;
        //    else
        //        pos.z = otherPos.z;
        //}
    }

    // update current tile
    _updateAdjTiles(disPos);
    // update for all adjacent tiles
    Array<mapPos> updates = _getNeighbours(disPos);
    for (int i = 0; i < updates.size(); i++)
        _updateAdjTiles(updates[i]);
    //for (int direc = 0; direc < 4; direc++)
    //    _updateAdjTiles(disPos + direcToVec[direc]);

    if (tile.type == CHECKPOINT_TILE) {
        lastCheckpointPosition = disPos;
        lastCheckpointStepsFromStart = stepsAwayFromStart;
    }
}

void Mapper::discover(bool front, bool left, bool right, bool back, bool up, uint8_t type) {
    if (_panic)
        return;

    discover(front, left, right, back, up, type, pos, rotation);
}

void Mapper::discover(bool front, bool left, bool right, bool back, bool up, uint8_t type, mapPos disPos, uint8_t disRotation) {
    if (_panic)
        return;

    // if type is a ramp type it is assumed that the rotation is the ramp direction        
    TileCon data(0, 0, 0, 0, 0, type, disRotation, up); // the ramp data will only be used if it is a ramp type
    data[disRotation] = front;
    data[(disRotation + 1) % 4] = right;
    data[(disRotation + 2) % 4] = back;
    data[(disRotation + 3) % 4] = left;

    discover(data, disPos);
}

void Mapper::turn(int8_t amount) {
    if (_panic)
        return;

    rotation = wrap(rotation + amount, 0, 4);
}

void Mapper::drive(bool forward) {
    if (_panic)
        return;

    pos = _getDriveInDirec(pos, (rotation + !forward * 2) % 4); // if not forward it shoud take the opposite direction
    stepsAwayFromStart++;
    actions.push_back(Action(pos, stepsAwayFromStart));
}

void Mapper::driveAmount(int8_t amount) {
    if (_panic)
        return;

    for (int i = 0; i < abs(amount); i++)
        drive(amount >= 0);
}

void Mapper::driveMove(Move move) {
    if (_panic)
        return;

    turn(move.rotation);
    driveAmount((int8_t)move.distance);
}

void Mapper::driveMoves(const Array<Move>& moves) {
    if (_panic)
        return;

    for (unsigned int i = 0; i < moves.size(); i++)
        driveMove(moves[i]);
}

void Mapper::resetToLastCheckpoint() {
    if (_panic)
        return;

    stepsAwayFromStart = lastCheckpointStepsFromStart;
    pos = lastCheckpointPosition;
    rotation = 0; //! Assumption that when the robot resets it is turned in the starting direction
}

uint8_t Mapper::goTo(mapPos endPos, mapPos startPos, Array<Move>* array, uint8_t startRotation) {
    if (_panic)
        return startRotation;

    LACK
    return _recursiveGoTo(endPos, _getAllStepsAway(endPos), startPos, array, startRotation);
}

uint8_t Mapper::goTo(mapPos endPos, Array<Move>* array) {
    LACK
    return goTo(endPos, pos, array, rotation);
}

Array<Move> Mapper::getNextMove(bool fWall, bool rWall, bool lWall, bool bWall) {
    if (_panic) { // if panic mode is active a drive left algorithm is initiated (in this case if not boxed in the maze will never be marked as complete)
        if (!lWall)
            return Array<Move>({Move(-1, 1)});
        else if (!fWall)
            return Array<Move>({Move(0, 1)});
        else if (!rWall)
            return Array<Move>({Move(1, 1)});
        else if (!bWall)
            return Array<Move>({Move(1, 0), Move(1, 1)});
        else
            return Array<Move>();
    }

    LACK;

    // this is a bit of a nitpick, but it would be more efficient to get all occourences of the current tile your on
    // and then search the neighbouring indexes or to be a bit more efficient, but more comliated use a Breadth-First Search
    VAR_PRINTLN(actions.size());
    for (int i = actions.size() - 1; i >= 0; i--) {
        VAR_PRINTLN(i);
        VAR_FUNC_PRINTLN(actions[i].pos);
        VAR_PRINTLN(map.getAdj(actions[i].pos));
        if (map.getAdj(actions[i].pos) == false)
            continue;
        if (map.getType(actions[i].pos) == BLACK_TILE)
            continue; // cannot go to a black tile
        Array<Move> moves;
        DB_PRINTLN("Valid tile!");
        VAR_FUNC_PRINTLN(actions[i].pos);
        actions[i].pos = _getLowerRampPos(actions[i].pos);
        VAR_FUNC_PRINTLN(actions[i].pos)
        //if (map.getType(actions[i].pos) == RAMP_TILE)
        //    actions[i].pos.z = 0; // updates the z position on a ramp tile or else it will try to go to a coordinate it can't get to
        LACK
        uint8_t endRoation = goTo(actions[i].pos, &moves);
        LACK
        for (unsigned int i = 0; i < moves.size(); i++) {
            moves[i].println();
        }
        uint8_t undiscoveredAdjTileDirec = _getUndiscAdjTileDirec(actions[i].pos, endRoation);
        VAR_PRINTLN(undiscoveredAdjTileDirec);
        if (undiscoveredAdjTileDirec == 5) {
            ERROR("current tile has been marked as having adj. undisc. tiles, but turned out to have none check tile updating!");
            undiscoveredAdjTileDirec = 0;
        }
        int8_t moveRotation = deltaRotation(endRoation, undiscoveredAdjTileDirec);
        if (moveRotation == 0 && moves.size() != 0)
            moves[moves.size() - 1].distance++;
        else {
            if (moveRotation == 2) {
                moves.push_back(Move(1, 0));
                moves.push_back(Move(1, 1));
            }
            else
                moves.push_back(Move(moveRotation, 1));
        }
        return moves;
    }
    return Array<Move>();
}

bool Mapper::hasAllreadySeenVictim(mapPos pos) {
    for (unsigned int i = 0; i < allreadySeenVictims.size(); i++) {
        if (allreadySeenVictims[i] == pos)
            return true;
    }
    return false;
}

void Mapper::addSeenVictim(mapPos pos) {
    allreadySeenVictims.push_back(pos);
}

void Mapper::panicMode() {
    _panic = true;
    DB_PRINT(SET_RED);
    DB_PRINT("MAPPER ENTERED PANIC MODE, ERROR OCCOURED!");
    DB_PRINTLN(RESET_COLOR);
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

// private methods are not edited for panic mode!

mapPos Mapper::_getDriveInDirec(mapPos currPos, uint8_t currDirec) {
    mapPos nextTile = currPos + direcToVec[currDirec];
    currPos = _getLowerRampPos(currPos); // ensures if on a ramp its on the lower one
    if (map.getType(currPos) == RAMP_TILE)
        if (map.getRampDir(currPos) == currDirec) // if on a ramp and facing the right direction move up
            nextTile.z += 1;
    currPos = _getLowerRampPos(nextTile); // _getLowerRampPos just in case we went onto a high tile ramp
    return currPos;
}

void Mapper::_updateAdjTiles(mapPos updPos) {
    DB_COLOR_PRINTLN("started _updateAdjTiles with: ", SET_RED);
    updPos.println();
    if (map.getType(updPos) == UNDISCOVERED_TILE) // no point in updating a undiscovered tile
        return;
    bool hasAdjUndiscoveredTiles = false;
    updPos = _getLowerRampPos(updPos);
    VAR_FUNC_PRINTLN(updPos);
    Array<mapPos> neighbours = _getNeighbours(updPos);
    VAR_PRINTLN(neighbours.size());
    for (unsigned int i = 0; i < neighbours.size(); i++) {
        VAR_PRINTLN(i);
        VAR_FUNC_PRINTLN(neighbours[i]);
        VAR_PRINTLN(map.getType(neighbours[i]));
        if (map.getType(neighbours[i]) != UNDISCOVERED_TILE)
            continue;
        hasAdjUndiscoveredTiles = true;
        break;
    }
    map.setAdj(updPos, hasAdjUndiscoveredTiles);
    VAR_PRINTLN(hasAdjUndiscoveredTiles);
    VAR_PRINTLN(map.getAdj(updPos));
    if (map.getType(updPos) == RAMP_TILE) { // update linked ramps to the same value
        map.setAdj(mapPos(updPos.x, updPos.y, updPos.z + 1), hasAdjUndiscoveredTiles);
        VAR_PRINTLN(map.getAdj(mapPos(updPos.x, updPos.y, updPos.z + 1)));
    }
}

Array<mapPos> Mapper::_getNeighbours(mapPos tilePos) {
    Array<mapPos> neighbours;
    tilePos = _getLowerRampPos(tilePos); // ensure on lower ramp
    if (map.getType(tilePos) == RAMP_TILE) {
        uint8_t rampDirec = map.getRampDir(tilePos);
        mapPos upPos = tilePos + direcToVec[rampDirec];
        upPos.z += 1;
        neighbours.push_back(upPos);
        neighbours.push_back(tilePos + direcToVec[(rampDirec + 2) % 4]);
        return neighbours;
    }
    for (uint8_t direc = 0; direc < 4; direc++) {
        if (map.getWall(tilePos, direc))
            continue;
        neighbours.push_back(tilePos + direcToVec[direc]);
    }
    return neighbours;
}

Array<uint16_t> Mapper::_getAllStepsAway(mapPos pos) {
    pos = _getLowerRampPos(pos);
    Array<uint16_t> stepsAway;
    for (uint16_t i = 0; i < actions.size(); i++) {
        actions[i].pos = _getLowerRampPos(actions[i].pos); //! CHECK IF THIS DOESN'T CAUSE MASSIVE TIME LOSS
        // because looking up the type and upadting it for each move is a bit accessive, but sometimes not really because its usefull
        if (actions[i].pos == pos)
            stepsAway.push_back(actions[i].stepsAway);
    }
    return stepsAway;
}

Move Mapper::_nextBestMoveTo(const Array<uint16_t>& endPosStepsAway, mapPos pos, uint8_t currRotation) {
    uint16_t minDist = 0xFFFF;
    Array<mapPos> neighbours = _getNeighbours(pos);
    unsigned int minIndex = neighbours.size();
    for (unsigned int i = 0; i < neighbours.size(); i++) {
        if (map.getType(neighbours[i]) == BLACK_TILE)
            continue; // basically the same thing as a wall
        uint16_t currDist = _minDistance(endPosStepsAway, _getAllStepsAway(neighbours[i]));
        if (currDist < minDist) {
            minDist = currDist;
            minIndex = i;
        }
    }
    if (minIndex == neighbours.size()) {
        ERROR("min direction couldn't be found (should be impossible)!");
        return Move(0, 0);
    }
    // no need to think about ramps, because vecToDirec sets the z value to 0 automatically
    return Move(deltaRotation(currRotation, vecToDirec(neighbours[minIndex] - pos)), 1);
}

uint16_t Mapper::_minDistance(const Array<uint16_t>& stepsAwayA, const Array<uint16_t>& stepsAwayB) {
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

uint8_t Mapper::_recursiveGoTo(mapPos endPos, const Array<uint16_t>& endPosStepsAway, mapPos startPos, Array<Move>* array, uint8_t startRotation) {
    if (startPos == endPos) {
        return startRotation;
    }
    Move thisMove = _nextBestMoveTo(endPosStepsAway, startPos, startRotation);
    if (thisMove.rotation == 0 && array->size() != 0)
        (*array)[array->size() - 1].distance++;
    else {
        Move splitMove = thisMove;
        if (abs(thisMove.rotation) == 2) {
            array->push_back(Move(1, 0));
            splitMove.rotation = 1;
        }
        array->push_back(splitMove);
    }
    uint8_t rot = wrap(startRotation + thisMove.rotation, 0, 4);
    mapPos nextPos = startPos;
    for (uint8_t move = 0; move < thisMove.distance; move++)
        nextPos = _getDriveInDirec(nextPos, rot);
    return _recursiveGoTo(endPos, endPosStepsAway, nextPos, array, rot);
}

uint8_t Mapper::_getUndiscAdjTileDirec(mapPos checkPos, uint8_t currentDirec) {
    DB_COLOR_PRINT("started _getUndiscoveredAdjTileDirec with: ", SET_RED);
    checkPos.println();
    checkPos = _getLowerRampPos(checkPos);
    VAR_FUNC_PRINTLN(checkPos);
    Array<mapPos> neighbours = _getNeighbours(checkPos);
    uint8_t leastPriority = 5;
    uint8_t bestDirec = 5;
    VAR_PRINTLN(neighbours.size());
    for (unsigned int i = 0; i < neighbours.size(); i++) {
        VAR_PRINTLN(i);
        VAR_FUNC_PRINTLN(neighbours[i]);
        if (map.getType(neighbours[i]) != UNDISCOVERED_TILE)
            continue;
        LACK;
        uint8_t direc = vecToDirec(neighbours[i] - checkPos);
        uint8_t relDirec = wrap(direc - currentDirec, 0, 4);
        if (direcCheckPriority[relDirec] < leastPriority) {
            leastPriority = direcCheckPriority[relDirec];
            bestDirec = direc;
        }
    }
    if (bestDirec == 5)
        ERROR("This function was used on a tile with no adj undiscovered tiles!");
    return bestDirec;
}

mapPos Mapper::_getLowerRampPos(mapPos checkPos) {
    uint8_t data = map.get(checkPos); // uses one get to be faster (just for lower level code)
    if (_GET_TYPE(data) != RAMP_TILE)
        return checkPos;
    if (_GET_UP(data)) // if the ramp goes up its the lower of the two
        return checkPos;
    else
        return mapPos(checkPos.x, checkPos.y, checkPos.z - 1);
}