#ifndef DISPLAY_CONF_H
#define DISPLAY_CONF_H

#include <Display/menu.h>
#include <ColorSensors/spectrometer.h>
#include <ColorSensors/rgbcSensor.h>
#include <devices.h>

extern Menu mainMenu;
extern Menu runMenu;
extern Menu calibMenu;
extern Menu inCalibrationMenu;
extern Menu testMenu;
extern Menu raspiMenu;
extern Menu startingRaspiMenu;

void mainFunc(Display* parentDisplay, Menu* parentMenu);
String getMappingPos();
void startRaspi(Display* parentDisplay, Menu* parentMenu);
String getCurrentVictim();

template <ColorType c>
void calibrateScript(Display* parentDisplay, Menu* parentMenu) {
    parentDisplay->currMenu = &inCalibrationMenu;
    parentDisplay->update();
    DB_PRINT_MUL((F("Calibrating "))(colorTypeToName[static_cast<uint8_t>(c)])(F("!\n")));
    Devices::spec.setColorCurrent(c);
    Devices::rgbcSensor.setColor(c);
    DB_PRINT_MUL((F("Finished calibrating "))(colorTypeToName[static_cast<uint8_t>(c)])(F("!\n")));
    parentDisplay->currMenu = &calibMenu;
    parentDisplay->update();
}

#endif // DISPLAY_CONF_H