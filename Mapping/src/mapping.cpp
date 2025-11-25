#include <mapping.h>

// global vars
// --------------------------------------------------

mapPos direcLookup[4] = {
    mapPos(0, -1), mapPos(1, 0), mapPos(0, 1), mapPos(-1, 0)
};

uint8_t tileWeights[6] = {
    255,    // UNDISCOVERED_TILE (should not happen)
    1,      // NORMAL_TILE
    5,      // BLUE_TILE
    255,    // BLACK_TILE (should also not happen)
    3,      // RAMP_TILE
    1       // CHECKPOINT_TILE
};

// --------------------------------------------------

Likely<Action> posDiffToAction(MapRotation rot, mapPos diff) {
    diff.z = 0;
    int8_t dir = -1;
    for (int i = 0; i < 4; i++) {
        if (direcLookup[i] == diff) {
            dir = i;
            break;
        }
    }
    if (dir == -1)
        return Likely<Action>(F("Invalid diff, hast to be a one tile difference!"));
    return Action(diffRotation(rot, dir), 1);
}

int8_t diffRotation(MapRotation a, MapRotation b) {
    int8_t diff = b.getRotation() - a.getRotation();
    diff = (diff + 4) % 4; // Normalize to range 0–3
    if (diff > 2) diff -= 4; // Convert 3 to -1 for shortest rotation
    return diff;
}


// ----------------------------------------------------------------------------------------------------
// class Mapper
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

void Mapper::discover(float risingAngle, bool front, bool back, bool left, bool right, uint8_t tileType) {
    discover(_rotation, _pos, risingAngle, front, back, left, right, tileType);
}

void Mapper::discover(MapRotation rot, mapPos pos, float risingAngle, bool front, bool back, bool left, bool right, uint8_t tileType) {
    bool ramp = risingAngle >= RISING_THREASHOLD || risingAngle <= -RISING_THREASHOLD;
    if (ramp) {
        bool up = risingAngle >= RISING_THREASHOLD;
        map.set(pos, TileCon(0, 0, 0, 0, true, RAMP_TILE, rot.getRotation(), up));
        map.set(mapPos(pos.x, pos.y, pos.z + (up ? 1 : -1)),
            TileCon(0, 0, 0, 0, true, RAMP_TILE, (rot + 2).getRotation(), !up));
        _updateAllAdjTiles(pos);
        return;
    }
    TileCon data(0, 0, 0, 0, true, tileType);
    data[rot.getRotation()] = front;
    data[(rot + 1).getRotation()] = right;
    data[(rot + 2).getRotation()] = back;
    data[(rot + 3).getRotation()] = left;
    map.set(pos, data);
    _updateAllAdjTiles(pos);
}

Likely<PrimArray<Action>> Mapper::goTo(mapPos origin, MapRotation rotation, mapPos end) {
    uint8_t endType = map.getType(end);
    if (endType == UNDISCOVERED_TILE)
        return Likely<PrimArray<Action>>(String(F("Could not get to end, beacuse it is undiscovered!")));
    if (endType == BLACK_TILE)
        return Likely<PrimArray<Action>>(String(F("Could not get to end, beacuse it is a black tile!")));

    LACK;

    Queue<PrimArray<Action>> queue;
    queue.appendData(PrimArray<Action>({}));

    LACK;

    PrimArray<Action> solution;
    uint16_t solutionTime = 0xFFFF;

    LACK;

    // we declare all variables before the loop, just to be a bit faster
    PrimArray<Action> currCandidate;
    mapPos currPos = origin;
    mapPos lastPos = origin;
    MapRotation currRotation = rotation;
    uint16_t candidateTime = 0;
    uint8_t data = 0;
    mapPos neighbors[4];
    uint8_t numNeighbors = 0;
    while (!queue.empty()) {
        currCandidate = queue.popFront().getData(); // we are popping front to do a breadth first search

        for (int i = 0; i < currCandidate.size(); i++)
            DB_PRINT_MUL((i)(": ")(currCandidate[i].getData()->rotation)(", ")(currCandidate[i].getData()->drive)('\n'));
        BREAK;

        // evaluate the current candidate
        currPos = origin;
        lastPos = origin;
        currRotation = rotation;
        candidateTime = 0;
        for (uint8_t i = 0; i < currCandidate.size(); i++) {
            lastPos = currPos;
            data = map.get(currPos);
            candidateTime += tileWeights[_GET_TYPE(data)];
            currCandidate[i].getData()->apply(currPos, currRotation, data);
        }
        data = map.get(currPos);
        candidateTime += tileWeights[_GET_TYPE(data)];

        TileCon(data).println();
        VAR_PRINTLN(candidateTime);
        BREAK;

        if (_GET_TYPE(data) == BLACK_TILE || _GET_TYPE(data) == UNDISCOVERED_TILE)
            continue; // we should not go outisde the currently discovered map and we can't go though black tiles
        
        if (candidateTime >= solutionTime)
            continue; // we allready have a better solution so we don't care
            
        if (currPos == end) { // we have found a solution that is better
            currCandidate.migrateTo(&solution); // uses migrateTo to avoid copying, because currCandidate is not used anymore
            solutionTime = candidateTime;
            continue;
        }

        // create new candidates with all of the new possible directions
        numNeighbors = getNeighbors(currPos, neighbors);
        DB_PRINTLN(F("Neighbors:"));
        for (int i = 0; i < numNeighbors; i++) {
            neighbors[i].println();
            map.println(neighbors[i]);
        }
        BREAK;
        for (int i = 0; i < numNeighbors; i++) {
            if (neighbors[i] == lastPos)
                continue; // avoid going back on yourself
            PrimArray<Action> newCandidate = currCandidate; // copy candidate
            Likely<Action> nextAction = posDiffToAction(currRotation, mapPos(neighbors[i].x - currPos.x, neighbors[i].y - currPos.y));
            if (nextAction.failed()) {
                String error =
                    String(F("Failed at ")) + String(__LINE__) + F(" because of posDiffToAction returning following Error:") + '\n' +
                    *nextAction.getError();
                return Likely<PrimArray<Action>>(error);
            }
            if (nextAction.getData().rotation == 0)
                newCandidate[newCandidate.size() - 1].getData()->drive += 1;
            else
                newCandidate.pushBack(nextAction.getData());
            queue.appendData(newCandidate);
        }
        DB_PRINTLN(F("All candidates:"));
        QueueLink<PrimArray<Action>>* curr = queue.getFirst();
        while (curr != nullptr)
        {
            for (int i = 0; i < curr->data.size(); i++)
                DB_PRINT_MUL(("(")(curr->data[i].getData()->rotation)(", ")(curr->data[i].getData()->drive)(") "));
            DB_PRINTLN();
            curr = curr->next;
        }
        BREAK;
    }

    if (solutionTime == 0xFFFF)
        return Likely<PrimArray<Action>>(String(F("Could not find a path to given end!")));

    return Likely<PrimArray<Action>>(solution);
}

uint8_t Mapper::getNeighbors(mapPos pos, mapPos* neighbors) {
    return getNeighbors(pos, map.get(pos), neighbors);
}

uint8_t Mapper::getNeighbors(mapPos pos, uint8_t data, mapPos* neighbors) {
    if (_GET_TYPE(data) == RAMP_TILE) {
        MapRotation dir = _GET_DIR(data);
        MapRotation otherDir = dir + 2;
        neighbors[0] = mapPos(
            pos.x + direcLookup[dir.getRotation()].x,
            pos.y + direcLookup[dir.getRotation()].x,
            pos.z + (_GET_UP(data) ? 1 : -1)
        );
        neighbors[1] = mapPos(
            pos.x + direcLookup[otherDir.getRotation()].x,
            pos.y + direcLookup[otherDir.getRotation()].x,
            pos.z
        );
        return 2;
    }

    uint8_t j = 0;
    for (uint8_t i = 0; i < 4; i++) {
        if (_GET_WALL(data, i))
            continue;
        neighbors[j] = mapPos(
            pos.x + direcLookup[i].x,
            pos.y + direcLookup[i].y,
            pos.z  
        );
        j++;
    }
    return j;
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

void Mapper::_updateAdjTilesBit(mapPos pos) {
    mapPos neighbors[4];
    uint8_t len = getNeighbors(pos, neighbors);
    for (int i = 0; i < len; i++) {
        uint8_t type = map.getType(neighbors[i]);
        if (type == UNDISCOVERED_TILE) {
            map.setAdj(pos, true);
            return;
        }
    }
    map.setAdj(pos, false);
}

void Mapper::_updateAllAdjTiles(mapPos pos) {
    mapPos neighbors[4];
    uint8_t len = getNeighbors(pos, neighbors);
    for (int i = 0; i < len; i++)
        _updateAdjTilesBit(neighbors[i]);
    _updateAdjTilesBit(pos);
}