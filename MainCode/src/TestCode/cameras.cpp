#include <Arduino.h>
#include <devices.h>
#include <RaspiComms/raspiDebug.h>

#define USE_camera true
#if CAT(USE_, CURR_MAIN)
#undef USE_camera

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);
    Serial1.begin(115200);

    DB_PRINTLN(F("Start!"));

    while (true)
    {
        if (Serial1.available())
            VAR_PRINTLN(Serial1.read());
        /*
        delay(20);
        // Devices::comms.sendTile(10, 11, 12, 100);
        // Devices::comms.sendPos(10, 10, 10, 3);

        switch (Devices::comms.update(100, 101, 102, 103)) {
        case RaspiEvent::CAMERA_INVALID:
            DB_PRINTLN(F("Invalid cam!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_LEFT:
            DB_PRINTLN(F("Cam triggered left!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_RIGTH:
            DB_PRINTLN(F("Cam triggered right!"));
            break;
        case RaspiEvent::DETECTED_H_RIGHT:
            DB_PRINTLN(F("DETECTED_H_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_H_LEFT:
            DB_PRINTLN(F("DETECTED_H_LEFT!"));
            break;
        case RaspiEvent::DETECTED_S_RIGHT:
            DB_PRINTLN(F("DETECTED_S_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_S_LEFT:
            DB_PRINTLN(F("DETECTED_S_LEFT!"));
            break;
        case RaspiEvent::DETECTED_U_RIGHT:
            DB_PRINTLN(F("DETECTED_U_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_U_LEFT:
            DB_PRINTLN(F("DETECTED_U_LEFT!"));
            break;
        case RaspiEvent::DETECTED_GREEN_RIGHT:
            DB_PRINTLN(F("DETECTED_GREEN_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_GREEN_LEFT:
            DB_PRINTLN(F("DETECTED_GREEN_LEFT!"));
            break;
        case RaspiEvent::DETECTED_YELLOW_RIGHT:
            DB_PRINTLN(F("DETECTED_YELLOW_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_YELLOW_LEFT:
            DB_PRINTLN(F("DETECTED_YELLOW_LEFT!"));
            break;
        case RaspiEvent::DETECTED_RED_RIGHT:
            DB_PRINTLN(F("DETECTED_RED_RIGHT!"));
            break;
        case RaspiEvent::DETECTED_RED_LEFT:
            DB_PRINTLN(F("DETECTED_RED_LEFT!"));
            break;
        case RaspiEvent::NO_MORE_PACKETS:
            break;
        case RaspiEvent::NONE:
            break;
        }
            */
    }
}

#endif