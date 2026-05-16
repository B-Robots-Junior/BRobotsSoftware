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

int main() {
    init();
    sei();
    BEGIN_DEBUG(BAUDE_RATE);

    if (!initEverything())
        ERROR(F("Could not init sensors!"));

    while (true) {
        gyro.update();

        DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)(F("roll: "))(ReadGyroroll())(F(", pitch: "))(ReadGyropitch())(F(", yaw: "))(ReadGyroyaw())('\n'));
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