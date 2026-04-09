#include <config.h>
#include <devices.h>

#define USE_calib true
#if CAT(USE_, CURR_MAIN)
#undef USE_calib

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    sei();
    Serial.begin(BAUDE_RATE);
    Devices::init();
    Devices::ledsBottom.fill(0xFF000000);
    Devices::ledsBottom.show();

    for (uint8_t type = 0; type < 5; type++) {
        Serial.print(SET_GREEN); Serial.print(F("Calibrate ")); Serial.print(colorTypeToName[type]); Serial.print('!'); Serial.print(RESET_COLOR); Serial.print('\n');
        do { Serial.print(SET_BLUE); Serial.print(F("Breakpoint in file: ")); Serial.print(F(__FILE__)); Serial.print(F(" in line: ")); Serial.print(__LINE__); Serial.println(F(" Triggered!")); Serial.print(RESET_COLOR); while (!Serial.available()) {} delay(100); while (Serial.available()) { Serial.read(); } } while (0);
        Devices::rgbcSensor.setColor(ColorType(type));
        Devices::spec.setColorCurrent(ColorType(type));
    }

    while(1) {}
}

#endif