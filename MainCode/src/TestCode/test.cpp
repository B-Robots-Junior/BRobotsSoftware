#include <Arduino.h>
#include <debug.h>
#include <PosSensors/ToF.h>
#include <PosSensors/position.h>

#include <config.h>

#define USE_test true
#if CAT(USE_, CURR_MAIN)
#undef USE_test

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    if (!initTofs())
        ERROR("Tof failed to init!");
    if (!initgyro())
        ERROR("Gyro failed to init!");

    while (1) {
        gyro.update();

        VAR_PRINTLN(ReadGyropitch());
    }
}

#endif