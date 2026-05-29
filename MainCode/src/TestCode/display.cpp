#include <debug.h>

#include <config.h>

#define USE_display true
#if CAT(USE_, CURR_MAIN)
#undef USE_display

#include <Adafruit_SSD1306.h>
// pins sind hier drinnen:
#include <pins.h>
#include <devices.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    BEGIN_DEBUG(BAUDE_RATE);
    sei();

    Devices::init();

    Devices::ledsTop.fill(0x40000000);
    Devices::ledsTop.show();
    Devices::ledsBottom.fill(0xFFFFFFFF);
    Devices::ledsBottom.show();

    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, DIS_MOSI_PIN, DIS_SCLK_PIN, DIS_DC_PIN, DIS_RST_PIN, DIS_CS_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC))
        ERROR(F("Could not begin display!"));

    display.clearDisplay();
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(3, 3);
    display.print("BRobots");
    
    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print(" Start >\n Calib >\n Test >\n Status >");

    display.setCursor(SCREEN_WIDTH / 2, 24);
    display.print("Tofs: OK");
    display.setCursor(SCREEN_WIDTH / 2, 32);
    display.print("Gyro: OK");
    display.setCursor(SCREEN_WIDTH / 2, 40);
    display.print("Color: OK");
    display.setCursor(SCREEN_WIDTH / 2, 48);
    display.print("Raspi: OK");

    display.display();

    while (1) {}
}

#endif