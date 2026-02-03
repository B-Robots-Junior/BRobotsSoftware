#include <Arduino.h>
#include <devices.h>
#include <RaspiComms/raspiDebug.h>

#define USE_camera true
#if CAT(USE_, CURR_MAIN)

int main() {
    init();

    while (true)
    {
        delay(500);
        Devices::comms.sendTile(10, 11, 12, 100);
        Devices::comms.sendPos(10, 10, 10, 3);

        switch (Devices::comms.update(100, 101, 102, 103)) {
        case RaspiEvent::CAMERA_INVALID:
            Devices::comms.debugLog(F("Invalid cam!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_LEFT:
            Devices::comms.debugLog(F("Cam triggered left!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_RIGTH:
            Devices::comms.debugLog(F("Cam triggered right!"));
            break;
        case RaspiEvent::DETECTED_OMEGA_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_OMEGA_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PHI_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PHI_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PSI_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PSI_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_0_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_0_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_1_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_1_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_2_LEFT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_2_RIGHT:
            Devices::comms.debugLog(F("Detected!"));
            break;
        case RaspiEvent::NO_MORE_PACKETS:
            break;
        case RaspiEvent::NONE:
            break;
        }
    }
}

#endif