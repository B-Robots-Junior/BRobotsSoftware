#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <RaspiComms/raspiComms.h>
#include <Display/display.h>
#include <ColorSensors/rgbcSensor.h>
#include <ColorSensors/refSensor.h>
#include <ColorSensors/spectrometer.h>
#include <Drive/Control.h>
#include <Adafruit_NeoPixel.h>
#include <pins.h>
#include <RescueSystem/RescuePackageHandler.h>

void rgbcSensorOnEnter();
void rgbcSensorOnExit();

class Devices {
public:
    static DualTB9051FTGMotorShield motors;
    static Control control;
    static RGBCSensor rgbcSensor;
    static RefSensor refSensor;
    static Spectrometer spec;
    static Adafruit_NeoPixel ledsBottom;
    static Adafruit_NeoPixel ledsTop;
    static RaspiComms comms;
    static RescuePackageHandler packageHandlerRight;
    static RescuePackageHandler packageHandlerLeft;
    // static Display display;

    static bool init(); // init all devices
};

#endif