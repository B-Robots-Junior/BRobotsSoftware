#include <U8g2lib.h>
#include <debug.h>

#include <config.h>

#define USE_display true
#if CAT(USE_, CURR_MAIN)
#undef USE_display

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

// Full buffer, hardware SPI
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI display(
    U8G2_R0,
    /* cs=*/53,
    /* dc=*/48,
    /* reset=*/U8X8_PIN_NONE
);

void setup() {
    BEGIN_DEBUG(BAUDE_RATE);
    DB_PRINTLN(F("Display Start!"));
    display.begin();
}

void loop() {
    display.clearBuffer();
    display.drawLine(0, 0, 127, 63);
    display.sendBuffer();
}

#endif