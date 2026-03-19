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

    while (1) {
        BREAK;
        DB_PRINTLN(F("--------------------------------------------------"));
        RACK;
        VAR_PRINTLN(mapper.map.dynamicMemorySize());
        VAR_FUNC_PRINTLN(mapper.pos);
        if (mapper.pos.x >= MAZE_WIDTH || mapper.pos.y >= MAZE_HEIGHT || mapper.pos.z >= MAZE_LAYERS || mapper.pos.x < 0 || mapper.pos.y < 0 || mapper.pos.z < 0)
            ERROR("Mapper went out of bounds!");
        VAR_PRINTLN(mapper.rotation);
        TileCon tile(test_maze[mapper.pos.z][mapper.pos.y][mapper.pos.x]);
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
        if (tile.type == BLACK_TILE)
            mapper.currMoveBlackTile(tile[mapper.rotation], tile[(mapper.rotation + 1) % 4], tile[(mapper.rotation + 3) % 4], tile[(mapper.rotation + 2) % 4]);
        else
            mapper.completeCurrMove();
    }
}