#include <Arduino.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <Util/utility.h>
#include <devices.h>

#include <config.h>

#define USE_turn true
#if CAT(USE_, CURR_MAIN)
#undef USE_turn

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    initTofs();
    gyro.begin();

    float startAngle = gyro.getYaw();
    uint32_t startTime = millis();

    while (true) {
        gyro.update();

        Devices::control.turnRobot(startAngle + 90, startTime, 5.0);
        /*DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)
                     (F("Current Angle: "))(gyro.getYaw())
                     (F("\nStart Angle: "))(startAngle)
                     (F("\nEnd Angle: "))(startAngle - 90)
                     (F("\nAngle diff: "))(angleDiffDEG(startAngle - 90, gyro.getYaw())));*/
    }
}

#endif