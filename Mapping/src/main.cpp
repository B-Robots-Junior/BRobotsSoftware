#include <map.h>
#include <debug.h>

int main() {
    init();

    BEGIN_DEBUG(9600);

    DB_PRINTLN("Start!");

    Map map;

    map.set(mapPos(0, 0, 0), 100);
    map.set(mapPos(10, 0, 0), 101);
    map.set(mapPos(0, 10, 0), 102);

    map.set(mapPos(10, 0, 10), 103);
    map.set(mapPos(-10, 5, 10), 104);

    map.set(mapPos(-10, 0, 0), 105);
    map.set(mapPos(0, -10, 0), 106);

    map.set(mapPos(10, 0, -10), 107);
    map.set(mapPos(14, 5, -10), 108);

    VAR_PRINTLN(map.get(mapPos(0, 0, 0)));
    VAR_PRINTLN(map.get(mapPos(10, 0, 0)));
    VAR_PRINTLN(map.get(mapPos(0, 10, 0)));

    VAR_PRINTLN(map.get(mapPos(10, 0, 10)));
    VAR_PRINTLN(map.get(mapPos(-10, 5, 10)));

    VAR_PRINTLN(map.get(mapPos(-10, 0, 0)));
    VAR_PRINTLN(map.get(mapPos(0, -10, 0)));

    VAR_PRINTLN(map.get(mapPos(10, 0, -10)));
    VAR_PRINTLN(map.get(mapPos(14, 5, -10)));

    while (1) {}
}