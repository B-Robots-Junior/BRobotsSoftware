#include <devices.h>
#include <pins.h>
#include <config.h>

RGBCSensor Devices::rgbcSensor(PIN_A12, rgbcSensorOnEnter, rgbcSensorOnExit);

RefSensor Devices::refSensor(33, PIN_A7, REF_SENSOR_OFFSET_EEPROM);

DualTB9051FTGMotorShield Devices::motors;

Control Devices::control(Devices::motors);

// RaspiComms Devices::comms(Serial);

// Display Devices::display(DIS_MOSI_PIN, DIS_SCLK_PIN, DIS_DC_PIN, DIS_RST_PIN, DIS_CS_PIN);

#define CHECK_INIT(x) do { if (!(x)) {worked = false; DB_PRINT_MUL((SET_RED)(F("Init of '"))(F(#x))("' in Devices::init Failed!\n")(RESET_COLOR));}} while (0)
bool Devices::init() {
    bool worked = true;
    CHECK_INIT(rgbcSensor.begin(new SoftwareWire(28, 27)));
    CHECK_INIT(refSensor.begin());
    motors.init();
    control.resetPIDs();
    return worked;
}
#undef CHECK_INIT