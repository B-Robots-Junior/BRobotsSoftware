#include <Arduino.h>
#include <debug.h>

#include <mapping.h>
#include <testMaze.h>

int main() {
    init();

    BEGIN_DEBUG(9600); // 115200
    RACK;

    VAR_PRINTLN(sizeof(void*));

    Mapper mapper;

    //! currently not working I think it has to do with Likely not playing well with PrimArray in the Queue return (maybe try without likely)
    RACK;
    Queue<PrimArray<float>> queue;
    queue.appendData(PrimArray<float>({1.0, 2.0, 3.0}));
    queue.appendData(PrimArray<float>({4.0, 5.0, 6.0}));
    queue.appendData(PrimArray<float>({7.0, 8.0, 9.0}));
    queue.appendData(PrimArray<float>({10.0, 11.0, 12.0}));
    QueueLink<PrimArray<float>>* currNode = queue.getFirst();
    RACK;
    while (currNode != nullptr) {
        DB_PRINT("{");
        for (int i = 0; i < currNode->data.size(); i++)
            DB_PRINT_MUL((*currNode->data[i].getData())(", "));
        DB_PRINT("}, ");
        currNode = currNode->next;
    }
    DB_PRINTLN();
    RACK;
    BREAK;
    Likely<PrimArray<float>> ldata = queue.popFront();
    if (ldata.failed())
        BREAK;
    DB_PRINTLN("Worked!");
    PrimArray<float> data = ldata.getData();
    BREAK;
    RACK;
    DB_PRINT("{");
    for (int i = 0; i < data.size(); i++)
        DB_PRINT_MUL((*data[i].getData())(", "));
    DB_PRINTLN("}");
    delay(1000);
    BREAK;
    data = queue.popFront().getData();
    BREAK;
    DB_PRINT("{");
    for (int i = 0; i < data.size(); i++)
        DB_PRINT_MUL((*data[i].getData())(", "));
    DB_PRINTLN("}");
    BREAK;
    currNode = queue.getFirst();
    while (currNode != nullptr) {
        DB_PRINT("{");
        for (int i = 0; i < currNode->data.size(); i++)
            DB_PRINT_MUL((*currNode->data[i].getData())(", "));
        DB_PRINT("}, ");
        currNode = currNode->next;
    }
    DB_PRINTLN();
    BREAK;

    DB_PRINTLN(F("started copying!"));

    // load entire maze to EEPROM
    for (int z = 0; z < MAZE_LAYERS; z++) {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                TileCon data = TileCon(maze[z][y][x]);
                if (data.type == UNDISCOVERED_TILE)
                    data.type = NORMAL_TILE;
                mapper.map.set(mapPos(x, y, z), data);
            }
        }
    }

    DB_PRINTLN(F("Finished copying!"));

    mapPos startPos = mapPos(0, 0, 0);
    MapRotation start = 0;
    mapPos endPos = mapPos(4, 4, 0);

    Likely<PrimArray<Action>> ret = mapper.goTo(startPos, start, endPos);
    if (ret.failed())
        DB_COLOR_PRINTLN(*ret.getError(), SET_RED);

    PrimArray<Action> array = ret.getData();
    for (int i = 0; i < array.size(); i++) {
        DB_PRINT_MUL((i)(": ")(array[i].getData()->rotation)(", ")(array[i].getData()->drive)('\n'));
    }

    while (1) {}
}