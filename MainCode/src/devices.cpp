#include <devices.h>
#include <pins.h>
#include <config.h>

RGBCSensor Devices::rgbcSensor(PIN_A12, rgbcSensorOnEnter, rgbcSensorOnExit);

RefSensor Devices::refSensor(33, PIN_A7, REF_SENSOR_OFFSET_EEPROM);

Spectrometer Devices::spec;

Adafruit_NeoPixel Devices::ledsBottom(5, NEOPIXEL_PIN_BOTTOM, NEO_GRBW + NEO_KHZ800);

Adafruit_NeoPixel Devices::ledsTop(28, NEOPIXEL_PIN_TOP, NEO_GRBW + NEO_KHZ800); // 14 per side

DualTB9051FTGMotorShield Devices::motors;

Control Devices::control(Devices::motors);

// Display Devices::display(DIS_MOSI_PIN, DIS_SCLK_PIN, DIS_DC_PIN, DIS_RST_PIN, DIS_CS_PIN);

RaspiComms Devices::comms(Serial1);

#define CHECK_INIT(x) do { if (!(x)) {worked = false; DB_PRINT_MUL((SET_RED)(F("Init of '"))(F(#x))(F("' in Devices::init Failed!\n"))(RESET_COLOR));}} while (0)
bool Devices::init() {
    bool worked = true;
    CHECK_INIT(rgbcSensor.begin(new SoftwareWire(28, 27)));
    CHECK_INIT(refSensor.begin());
    CHECK_INIT(ledsBottom.begin());
    CHECK_INIT(ledsTop.begin());
    CHECK_INIT(spec.begin());
    // CHECK_INIT(display.begin());
    Serial1.begin(115200);
    motors.init();
    control.resetPIDs();
    return worked;
}
#undef CHECK_INIT