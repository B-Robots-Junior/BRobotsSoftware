#include <Arduino.h>
#include <debug.h>
#include <ColorSensors/spectrometer.h>
#include <Adafruit_NeoPixel.h>
#include <devices.h>

#include <config.h>

#define USE_auswurf true
#if CAT(USE_, CURR_MAIN)
#undef USE_auswurf

#define ENDLINE F("                                                 \n")

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    Devices::init();

    initEncoders();

    // Devices::motors.setSpeeds(100, 100, 100, 100);

    Devices::packageHandlerRight.begin();
    Devices::packageHandlerLeft.begin();

    // Devices::packageHandlerRight.trigger(2);
    // Devices::packageHandlerLeft.trigger(2);

    DB_PRINT(CLEAR_SCREEN_AND_HOME);

    while (true) {
        Devices::packageHandlerRight.update();
        Devices::packageHandlerLeft.update();
        DB_PRINT_MUL((CURSOR_HOME)(getEncoderValueMM())(ENDLINE)(F("Trigger Package Handlers (r / l / a / A)?"))(ENDLINE)(ENDLINE));
        if (Serial.available() != 0) {
            char input = Serial.read();
            if (input == 'r' || input == 'a')
                Devices::packageHandlerRight.trigger(1);
            if (input == 'l' || input == 'a')
                Devices::packageHandlerLeft.trigger(1);
            if (input == 'A') {
                Devices::packageHandlerRight.trigger(6);
                Devices::packageHandlerLeft.trigger(6);
            }
            delay(200);
            while (Serial.available() != 0) { Serial.read(); }
        }
    }
}

#endif