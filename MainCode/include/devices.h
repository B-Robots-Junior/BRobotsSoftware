#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <RaspiComms/raspiComms.h>
#include <Display/display.h>
#include <ColorSensors/rgbcSensor.h>
#include <ColorSensors/refSensor.h>
#include <Drive/Control.h>

void rgbcSensorOnEnter();
void rgbcSensorOnExit();

class Devices {
public:
    static DualTB9051FTGMotorShield motors;
    static Control control;
    static RGBCSensor rgbcSensor;
    static RefSensor refSensor;
    // static RaspiComms comms;
    // static Display display; // currently not on there

    static bool init(); // init all devices
};

#endif