#include <Arduino.h>
#include <debug.h>
#include <PosSensors/ToF.h>
#include <PosSensors/position.h>
#include <devices.h>

#include <config.h>

#define USE_tof_calib true
#if CAT(USE_, CURR_MAIN)
#undef USE_tof_calib

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    Devices::ledsTop.begin();
    Devices::ledsTop.fill(0x40000000);
    Devices::ledsTop.show();

    Wire.begin();

    if (!initTofs())
        ERROR("Tof failed to init!");
    if (!initgyro())
        ERROR("Gyro failed to init!");

    uint64_t sum = 0;
    for (int i = 0; i < 100; i++) {
        updateTofs();
        DB_PRINTLN(getRBDistance());
        sum += getRBDistance();
        delay(10);
    }
    VAR_PRINTLN(sum / 100.0);

    while (1) {}
}

#endif