#include <map.h>
#include <mapping.h>
#include <debug.h>
#include <test.h>

int main() {
    init();

    BEGIN_DEBUG(115200);

    RACK;
    DB_PRINTLN("Start!");

    Mapper mapper;

    int offsetX = 4;
    int offsetY = 0;
    int offsetZ = 0;

    while (1) {
        /*if (INPUT_BOOL((F("Reset to checkpoint ")), false)) {
            uint8_t data = mapper.map.get(mapper._actions[mapper._lastCheckpointActionIndex]);
            mapper.resetToLastCheckpoint(
                _GET_NORTH(data),
                _GET_EAST(data),
                _GET_WEST(data),
                _GET_SOUTH(data)
            );
            mapper.map.println(mapper.pos);
            VAR_PRINTLN(mapper.map.getAdj(mapper.pos));
        }*/
        DB_PRINTLN(F("--------------------------------------------------"));
        BREAK;
        RACK;
        VAR_PRINTLN(mapper.map.dynamicMemorySize());
        VAR_FUNC_PRINTLN(mapper.pos);
        if (mapper.pos.x + offsetX >= MAZE_WIDTH || mapper.pos.y + offsetY >= MAZE_HEIGHT || mapper.pos.z + offsetZ >= MAZE_LAYERS
            || mapper.pos.x + offsetX < 0 || mapper.pos.y + offsetY < 0 || mapper.pos.z + offsetZ < 0)
            ERROR("Mapper went out of bounds!");
        VAR_PRINTLN(mapper.rotation);
        TileCon tile(test_maze[mapper.pos.z + offsetZ][mapper.pos.y + offsetY][mapper.pos.x + offsetX]);
        DB_PRINTLN(F("Current Tile:"));
        tile.println();
        Move mov = mapper.currMove(
            tile[mapper.rotation],
            tile[(mapper.rotation + 1) % 4],
            tile[(mapper.rotation + 3) % 4],
            tile[(mapper.rotation + 2) % 4],
            tile.type == RAMP_TILE ? tile.up : false,
            tile.type 
        );
        DB_PRINTLN(F("Current Tile Mapper"));
        mapper.map.println(mapper.pos);
        DB_PRINTLN(F("Current Move:"));
        mov.println();
        if (mov.rotation == 0 && mov.distance == 0) {
            DB_PRINT_MUL((SET_GREEN)(F("Completed maze!"))(RESET_COLOR));
            while (1) {}
        }
        mapPos posAfterMove = mapper._getDriveInDirec(mapper.pos, (mapper.rotation + mov.rotation + 4) % 4);
        VAR_FUNC_PRINTLN(posAfterMove);
        TileCon tileAfterMove(test_maze[posAfterMove.z + offsetZ][posAfterMove.y + offsetY][posAfterMove.x + offsetX]);
        if (tileAfterMove.type == BLACK_TILE) {
            DB_PRINTLN(F("Triggered black tile move!"));
            mapper.currMoveBlackTile(
                tileAfterMove[mapper.rotation],
                tileAfterMove[(mapper.rotation + 1) % 4],
                tileAfterMove[(mapper.rotation + 3) % 4],
                tileAfterMove[(mapper.rotation + 2) % 4]
            );
            VAR_FUNC_PRINTLN(mapper.pos);
        }
        else
            mapper.completeCurrMove();
    }
}