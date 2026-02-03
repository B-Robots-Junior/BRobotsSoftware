#include <Display/display.h>

// ----------------------------------------------------------------------------------------------------
// Display class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

bool Display::begin() {
    if (_good)
        return true;
    _good = display.begin(SSD1306_SWITCHCAPVCC);
    return _good;
}

void Display::update() {
    uint32_t displayTimeStamp = millis();
    currMenu->update(this);
    displayTimeStamp = millis();
    display.clearDisplay();
    if (currMenu != nullptr)
        currMenu->draw(this);
    displayTimeStamp = millis();
    display.display();
    if (_runCurrSelectableOnNextUpdate) {
        _runCurrSelectableOnNextUpdate = false;
        runCurrentSelectable();
    }
}

void Display::runCurrentSelectable() {
    currMenu->runCurrSelectable(this);
}

void Display::print(const String& string, DisPos pos, uint8_t size, uint16_t color) {
    String str = string;
    display.setTextSize(size);
    display.setTextColor(color);
    display.setCursor(pos.x, pos.y);
    //LACK;
    //VAR_PRINTLN(size);
    //VAR_PRINTLN(string.c_str());
    //VAR_PRINTLN(string.length());
    //LACK;
    display.print(str);
    //LACK;
}

void Display::fill(DisPos pos, int16_t width, int16_t height, uint16_t color) {
    display.fillRect(pos.x, pos.y, width, height, color);
}
