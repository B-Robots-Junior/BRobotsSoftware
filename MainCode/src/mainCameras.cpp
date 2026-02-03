#include <Arduino.h>
#include <raspiComms.h>

int main() {
    init();

    RaspiComms raspi(Serial);

    //DB_PRINTLN(F("Start"));

    while (true)
    {
        switch (raspi.update(100, 101, 102, 103)) {
        case RaspiEvent::CAMERA_INVALID:
            raspi.debugLog(F("Invalid cam!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_LEFT:
            raspi.debugLog(F("Cam triggered left!"));
            break;
        case RaspiEvent::CAMERA_TRIGGERED_RIGTH:
            raspi.debugLog(F("Cam triggered right!"));
            break;
        case RaspiEvent::DETECTED_OMEGA_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_OMEGA_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PHI_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PHI_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PSI_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_PSI_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_0_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_0_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_1_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_1_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_2_LEFT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::DETECTED_RING_SUM_2_RIGHT:
            raspi.debugLog(F("Detected!"));
            break;
        case RaspiEvent::NO_MORE_PACKETS:
            break;
        case RaspiEvent::NONE:
            break;
        }
    }
}