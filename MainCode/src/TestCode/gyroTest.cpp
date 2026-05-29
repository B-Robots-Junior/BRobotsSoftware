#include <config.h>
#include <devices.h>
#include <debug.h>
#include <PosSensors/position.h>

#define USE_gyro true
#if CAT(USE_, CURR_MAIN)
#undef USE_gyro

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

bool initEverything();

#define ENDLINE F("                                                 \n")

int main() {
    init();
    sei();
    BEGIN_DEBUG(BAUDE_RATE);

    if (!initEverything())
        ERROR(F("Could not init sensors!"));

    DB_PRINT(CLEAR_SCREEN);

    while (true) {
        gyro.update();

        DB_PRINT_MUL((CURSOR_HOME)(F("roll: "))(ReadGyroroll())(F(", pitch: "))(ReadGyropitch())(F(", yaw: "))(ReadGyroyaw())(ENDLINE));
        delay(100);
    }
    
}

#define CHECK_INIT(x) do { if (!(x)) {worked = false; DB_PRINT_MUL((SET_RED)(F("Init of '"))(F(#x))("' Failed!\n")(RESET_COLOR));}} while (0)
bool initEverything() {
    bool worked = true;
    CHECK_INIT(initEncoders());
    CHECK_INIT(initTofs());
    CHECK_INIT(initgyro());
    CHECK_INIT(Devices::init());
    return worked;
}
#undef CHECK_INIT

#endif