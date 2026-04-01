#include <Arduino.h>
#include <debug.h>
#include <PosSensors/ToF.h>
#include <PosSensors/position.h>

#include <config.h>

#define USE_tof_calib true
#if CAT(USE_, CURR_MAIN)
#undef USE_tof_calib

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    if (!initTofs())
        ERROR("Tof failed to init!");
    if (!initgyro())
        ERROR("Gyro failed to init!");

    uint64_t sum = 0;
    for (int i = 0; i < 100; i++) {
        updateTofs();
        DB_PRINTLN(getBackDistance());
        sum += getBackDistance();
        delay(10);
    }
    VAR_PRINTLN(sum / 100.0);

    while (1) {}
}

#endif