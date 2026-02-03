#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <RaspiComms/raspiComms.h>
#include <Display/display.h>

class Devices {
public:
    static RaspiComms comms;
    static Display display;
};

#endif