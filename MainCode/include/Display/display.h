#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Adafruit_SSD1306.h>
#include <Display/menu.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DEFAULT_FONT_SIZE 1

#define _WIDTH_PER_TEXT_SIZE 6 // I think this is right (check it tho)
#define _HEIGHT_PER_TEXT_SIZE 8

#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
#define INVERT SSD1306_INVERSE

class Menu;

typedef Pos<int8_t> DisPos;

class Display {
private:
    bool _good = false;
    uint8_t _width, _height;

    bool _runCurrSelectableOnNextUpdate = false;

public:
    Display(uint8_t mosi_pin, uint8_t sclk_pin, uint8_t dc_pin, uint8_t rst_pin, uint8_t cs_pin, uint8_t width = SCREEN_WIDTH, uint8_t height = SCREEN_HEIGHT) : 
        _width(width), _height(height), display(width, height, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin) {}

    Menu* currMenu;
    Adafruit_SSD1306 display;

    bool begin();

    void update();

    bool good() const {return _good;}

    uint8_t width() const {return _width;}
    uint8_t height() const {return _height;}

    void runCurrentSelectable();
    void runCurrentSelectableOnNextUpdate() {_runCurrSelectableOnNextUpdate = true;}

    void print(const String& string, DisPos pos = DisPos(0, 0), uint8_t size = DEFAULT_FONT_SIZE, uint16_t color = WHITE);
    void fill(DisPos pos = DisPos(0, 0), int16_t width = SCREEN_WIDTH, int16_t height = SCREEN_HEIGHT, uint16_t color = BLACK);

    void clearDisplay() {display.clearDisplay();}
    void show() {display.display();}
};

#endif