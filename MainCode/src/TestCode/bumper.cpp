#include <Arduino.h>
#include <ColorSensors/spectrometer.h>
#include <debug.h>
#include <PosSensors/position.h>
#include <i2cScanner.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>
#include <Encoders/encoders.h>
#include <devices.h>

#include <config.h>

#define USE_bumper true
#if CAT(USE_, CURR_MAIN)
#undef USE_bumper

void rgbcSensorOnEnter() {
}

void rgbcSensorOnExit() {
}

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    initTofs();
    initgyro();
    initEncoders();
    Devices::init();

    /*
    Devices::motors.setSpeeds(BASE_SPEED_DRIVE, 0, 0, 0); // lf
    delay(5000);
    Devices::motors.setSpeeds(0, BASE_SPEED_DRIVE, 0, 0); // lb
    delay(5000);
    Devices::motors.setSpeeds(0, 0, BASE_SPEED_DRIVE, 0); // rf
    delay(5000);
    Devices::motors.setSpeeds(0, 0, 0, BASE_SPEED_DRIVE); // rb
    delay(5000);
    Devices::motors.setSpeeds(0, 0, 0, 0);
    */

    uint32_t startTime = millis();

    while (true) {
        if (Devices::control.driveSCurve(100, 50, startTime, 500, -1) == 0)
            while (true) {}
    }
}

#endif