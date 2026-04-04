#include <Arduino.h>
#include <ColorSensors/spectrometer.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <i2cScanner.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>

#include <config.h>

#define USE_spec true
#if CAT(USE_, CURR_MAIN)
#undef USE_spec

void rgbcSensorOnEnter() {
}

void rgbcSensorOnExit() {
}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    pinMode(33, OUTPUT);

    digitalWrite(33, HIGH);

    while (true) {

    }
}

#endif