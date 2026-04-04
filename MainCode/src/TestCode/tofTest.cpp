#include <Arduino.h>
#include <debug.h>
#include <PosSensors/position.h>

#include <config.h>

#define USE_tof true
#if CAT(USE_, CURR_MAIN)
#undef USE_tof

void rgbcSensorOnEnter() {
}

void rgbcSensorOnExit() {
}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    initTofs();

    while (true) {
        DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)
            (F("Data:\n"))
            (F("    fl: "))(getFrontBottomShortDistance())(F(", f: "))(getFrontTopDistance())(F(", fr: "))(getFrontBottomLongDistance())('\n')
            (F("    lf: "))(getLFDistance())(F(", rf: "))(getRFDistance())('\n')
            (F("    lb: "))(getLBDistance())(F(", rb: "))(getRBDistance())('\n')
            (F("    b: "))(getBackDistance())('\n')
        );
    }
}

#endif