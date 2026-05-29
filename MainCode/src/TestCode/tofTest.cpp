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

bool rampInfront() {
    if (!getTofFTValid() || !getTofFBRValid())
        return false;
    return getFrontAngle() <= FRONT_RAMP_THRESHOLD;
}

#define ENDLINE F("                                                 \n")

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    initTofs();

    DB_PRINT(CLEAR_SCREEN_AND_HOME);

    while (true) {
        DB_PRINT_MUL((CURSOR_HOME)
            (F("Data:\n"))
            (F("    fl: "))(getFrontBottomShortDistance())(' ')(getTofFBLValid())(F(", f: "))(getFrontTopDistance())(' ')(getTofFTValid())(F(", fr: "))(getFrontBottomLongDistance())(' ')(getTofFBRValid())(ENDLINE)
            (F("    lf: "))(getLFDistance())(' ')(getTofLFValid())(F(", rf: "))(getRFDistance())(' ')(getTofRFValid())(ENDLINE)
            (F("    lb: "))(getLBDistance())(' ')(getTofLBValid())(F(", rb: "))(getRBDistance())(' ')(getTofRBValid())(ENDLINE)
            (F("    b: "))(getBackDistance())(' ')(getTofBValid())(ENDLINE)(ENDLINE)
            (F("    front: "))(wallFront())(ENDLINE)
            (F("    left: "))(wallLeft())(F(" right: "))(wallRight())(ENDLINE)
            (F("    back: "))(wallBack())(ENDLINE)(ENDLINE)
            (F("    frontAngle: "))(getFrontAngle())(F(", isRamp: "))(rampInfront())(ENDLINE)(ENDLINE)
        );
        delay(100);
    }
}

#endif