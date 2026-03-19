#include <Arduino.h>
#include <debug.h>
#include <ColorSensors/spectrometer.h>
#include <Adafruit_NeoPixel.h>

#include <config.h>

#define USE_reflectance true
#if CAT(USE_, CURR_MAIN)
#undef USE_reflectance

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    //Adafruit_NeoPixel leds(5, );

    Spectrometer spec;

    spec.begin();

    while (true) {

    }
}

#endif