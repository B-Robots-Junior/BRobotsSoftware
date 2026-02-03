#include <devices.h>
#include <pins.h>

RaspiComms Devices::comms(Serial);

Display Devices::display(DIS_MOSI_PIN, DIS_SCLK_PIN, DIS_DC_PIN, DIS_RST_PIN, DIS_CS_PIN);