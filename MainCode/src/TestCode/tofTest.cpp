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
            (F("    fl: "))(getFrontBottomShortDistance())(' ')(getTofFBLValid())(F(", f: "))(getFrontTopDistance())(' ')(getTofFTValid())(F(", fr: "))(getFrontBottomLongDistance())(' ')(getTofFBRValid())('\n')
            (F("    lf: "))(getLFDistance())(' ')(getTofLFValid())(F(", rf: "))(getRFDistance())(' ')(getTofRFValid())('\n')
            (F("    lb: "))(getLBDistance())(' ')(getTofLBValid())(F(", rb: "))(getRBDistance())(' ')(getTofRBValid())('\n')
            (F("    b: "))(getBackDistance())(' ')(getTofBValid())('\n')('\n')
            (F("    front: "))(wallFront())('\n')
            (F("    left: "))(wallLeft())(F(" right: "))(wallRight())('\n')
            (F("    back: "))(wallBack())('\n')('\n')
        );
    }
}

#endif