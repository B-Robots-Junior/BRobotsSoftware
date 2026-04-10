#include <devices.h>
#include <debug.h>

#include <config.h>

#define USE_drive true
#if CAT(USE_, CURR_MAIN)
#undef USE_drive

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    BEGIN_DEBUG(BAUDE_RATE);

    VAR_PRINTLN(digitalPinToTimer(6));
    VAR_PRINTLN(digitalPinToTimer(7));
    VAR_PRINTLN(digitalPinToTimer(8));
    VAR_PRINTLN(digitalPinToTimer(11));

    Devices::init();

    Devices::motors.init();
    Devices::motors.setSpeeds(100, 100, 100, 100);

    while (true) {

    }
}

#endif