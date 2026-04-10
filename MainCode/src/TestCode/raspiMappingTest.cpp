#include <Arduino.h>
#include <ColorSensors/spectrometer.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <i2cScanner.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>
#include <devices.h>

#include <config.h>

#define USE_raspi_mapping true
#if CAT(USE_, CURR_MAIN)
#undef USE_raspi_mapping

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    delay(100);

    Devices::comms.sendTile(0, 0, 0, 0b11010001);
    Devices::comms.sendTile(0, 1, 0, 0b01110001);

    while (true) {
        Devices::comms.update(0, 0, 0, 0);
    }
}

#endif