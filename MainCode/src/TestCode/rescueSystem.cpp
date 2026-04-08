#include <Arduino.h>
#include <ColorSensors/spectrometer.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <i2cScanner.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>
#include <Encoders/encoders.h>
#include <devices.h>
#include <RescueSystem/RescuePackageHandler.h>

#include <config.h>

#define USE_rescue true
#if CAT(USE_, CURR_MAIN)
#undef USE_rescue

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

#define SERVO1 44
#define SERVO2 13 // war 5, ist jetzt umgelötet

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    DB_PRINTLN(F("Start"));
    
    RescuePackageHandler packageHandlerRight(SERVO1, 45, 70);
    RescuePackageHandler packageHandlerLeft(SERVO2, 128, -70);

    packageHandlerRight.trigger(5);
    packageHandlerLeft.trigger(5);

    while (true) {
        packageHandlerRight.update();
        packageHandlerLeft.update();
    }
}

#endif