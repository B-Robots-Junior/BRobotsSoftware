#include <debug.h>

#include <config.h>

#define USE_display true
#if CAT(USE_, CURR_MAIN)
#undef USE_display

#include <Adafruit_SSD1306.h>
// pins sind hier drinnen:
#include <pins.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    BEGIN_DEBUG(BAUDE_RATE);
    sei();

    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, DIS_MOSI_PIN, DIS_SCLK_PIN, DIS_DC_PIN, DIS_RST_PIN, DIS_CS_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC))
        ERROR(F("Could not begin display!"));

    display.clearDisplay();
    display.drawLine(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.drawLine(0, SCREEN_HEIGHT, SCREEN_WIDTH, 0, SSD1306_WHITE);
    display.display();

    while (1) {}
}

#endif