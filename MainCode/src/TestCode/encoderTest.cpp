#include <config.h>
#include <devices.h>
#include <Encoders/encoders.h>

#define USE_encoderTest true
#if CAT(USE_, CURR_MAIN)
#undef USE_encoderTest

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

void triggerPackageThrow(RaspiEvent detection) {
    switch (detection)
    {
    case RaspiEvent::NONE:
    case RaspiEvent::NO_MORE_PACKETS:
    case RaspiEvent::CAMERA_TRIGGERED_RIGTH:
    case RaspiEvent::CAMERA_TRIGGERED_LEFT:
    case RaspiEvent::CAMERA_INVALID: break;
#if USE_NEW_RASPI_COMMS
    case RaspiEvent::DETECTED_PSI_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_PHI_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_OMEGA_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_0_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_1_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_2_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_PSI_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_PHI_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_OMEGA_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_0_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_1_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_2_LEFT: Devices::packageHandlerLeft.trigger(2); break; 
#else
    case RaspiEvent::DETECTED_H_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_S_RIGHT: Devices::packageHandlerRight.trigger(1); break;
    case RaspiEvent::DETECTED_U_RIGHT: break;
    case RaspiEvent::DETECTED_H_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_S_LEFT: Devices::packageHandlerLeft.trigger(1); break;
    case RaspiEvent::DETECTED_U_LEFT: break;
    case RaspiEvent::DETECTED_GREEN_RIGHT: break;
    case RaspiEvent::DETECTED_YELLOW_RIGHT: Devices::packageHandlerRight.trigger(1); break;
    case RaspiEvent::DETECTED_RED_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_GREEN_LEFT: break;
    case RaspiEvent::DETECTED_YELLOW_LEFT: Devices::packageHandlerLeft.trigger(1); break;
    case RaspiEvent::DETECTED_RED_LEFT: Devices::packageHandlerLeft.trigger(2); break;
#endif
    }
}

int main() {
    init();
    sei();
    BEGIN_DEBUG(BAUDE_RATE);
    Devices::init();
    
    initEncoders();
    triggerPackageThrow(RaspiEvent::DETECTED_H_RIGHT);
    triggerPackageThrow(RaspiEvent::DETECTED_H_LEFT);

    while(1) {
        DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)(F("Encoders: "))(getEncoderValueMM()));
        Devices::packageHandlerRight.update();
        Devices::packageHandlerLeft.update();
    }
}

#endif