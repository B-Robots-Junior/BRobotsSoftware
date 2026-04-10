#include <Arduino.h>
#include <debug.h>
#include <ColorSensors/spectrometer.h>
#include <Adafruit_NeoPixel.h>
#include <devices.h>

#include <config.h>

#define USE_auswurf true
#if CAT(USE_, CURR_MAIN)
#undef USE_auswurf

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    Devices::init();

    initEncoders();

    Devices::motors.setSpeeds(100, 100, 100, 100);

    Devices::packageHandlerRight.begin();
    Devices::packageHandlerLeft.begin();

    Devices::packageHandlerRight.trigger(2);
    Devices::packageHandlerLeft.trigger(2);

    while (true) {
        Devices::packageHandlerRight.update();
        Devices::packageHandlerLeft.update();
        DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)(getEncoderValueMM()));
    }
}

#endif