#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <Arduino.h>
#include <pos.h>
#include <constants.h>
#include <Display/display.h>
#include <Display/menu.h>
#include <devices.h>
#include <Util/utility.h>

#define DEFAULT_FONT_SIZE 1

#define WALL_SCETCH_SCALE 0.2

class Display;
class Menu;

typedef Pos<int8_t> DisPos;

// ----------------------------------------------------------------------------------------------------
// Element classes
// ----------------------------------------------------------------------------------------------------

// class to inherit from when making a custom element
class Element {
public:
    Element() {}

    virtual void draw(Display* parentDisplay) = 0;
};

class Rect : public Element {
public:
    Rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color = WHITE, bool fill = false)
        : _x(x), _y(y), _w(w), _h(h), _color(color), _fill(fill) {}

    void setX(uint8_t x) { _x = x; }
    void setY(uint8_t y) { _y = y; }
    void setW(uint8_t w) { _w = w; }
    void setH(uint8_t h) { _h = h; }
    void setColor(uint16_t color) { _color = color; }
    void setFill(bool fill) { _fill = fill; }

    uint8_t getX() const { return _x; } 
    uint8_t getY() const { return _y; } 
    uint8_t getW() const { return _w; } 
    uint8_t getH() const { return _h; } 
    uint16_t getColor() const { return _color; } 
    bool getFill() const { return _fill; } 

    void draw(Display* parentDisplay) override {
        if (_fill)
            parentDisplay->fill(DisPos(_x, _y), _w, _h, _color);
        else
            parentDisplay->display.drawRect(_x, _y, _w, _h, _color);
    }

protected:
    uint8_t _x;
    uint8_t _y;
    uint8_t _w;
    uint8_t _h;
    uint16_t _color;
    bool _fill;
};

template <int8_t x, int8_t y, uint8_t size>
class ConstText : public Element {
public:
    ConstText(const __FlashStringHelper* text) : Element(), text(text) {}

    const __FlashStringHelper* text;

    void draw(Display* parentDisplay) override {
        parentDisplay->print(text, DisPos(x, y), size, WHITE);
    }
};

#define GEN_CONST_TEXT(name, posX, posY, text, size) \
    const char name##Text[] PROGMEM = text; \
    ConstText<posX, posY, size> name(reinterpret_cast<const __FlashStringHelper*>(name##Text));

template <int8_t x, int8_t y, uint8_t size>
class FuncText : public Element {
public:
    FuncText(String (*func)()) : Element(), func(func) {}

    String (*func)();

    void draw(Display* parentDisplay) override {
        parentDisplay->print(func(), DisPos(x, y), size, WHITE);
    }
};

#define GEN_FUNC_TEXT(name, posX, posY, func, size) \
    FuncText<posX, posY, size> name(func);

template <int8_t x, int8_t y, uint8_t size>
class SensorReading : public Element {
public:
    SensorReading(const __FlashStringHelper* readingName, String (*getReading)(void)) 
        : readingName(readingName), getReading(getReading) {}

    const __FlashStringHelper* readingName;
    String (*getReading)(void);

    void draw(Display* parentDisplay) override {
        parentDisplay->print(String(readingName) + ": " + getReading(), DisPos(x, y), size, WHITE);
    }
};

#define GEN_SENSOR_READING(name, posX, posY, readingName, func, size) \
    const char name##ReadingName[] PROGMEM = readingName; \
    SensorReading<posX, posY, size> name(reinterpret_cast<const __FlashStringHelper*>(name##ReadingName), func);

template <uint8_t x, uint8_t y, uint8_t size, uint8_t pin>
class BatteryCheck : public ConstText<x, y, size> {
public:
    BatteryCheck(const __FlashStringHelper* text, Menu* nextMenu) : ConstText<x, y, size>(text), nextMenu(nextMenu) {}

    Menu* nextMenu;

    void draw(Display* parentDisplay) override {
        pinMode(pin, INPUT);
        uint16_t adcValue = analogRead(pin); // 0 bis 1023
        float voltageAtPin = (adcValue / 1023.0) * BATTERY_REF_VOLT; // Spannung nach dem Teiler (max. 4.5V)
        float originalVoltage = voltageAtPin * BATTERY_VOLT_DIVIDER_FACTOR;
        if (originalVoltage > 6) // battery exists, so continue to next menu
            parentDisplay->currMenu = nextMenu;
        else
            ConstText<x, y, size>::draw(parentDisplay);
    }
};

#define GEN_BATTERY_CHECK(name, posX, posY, size, pin, text, nextMenu) \
    const char name##BatteryText[] PROGMEM = text; \
    BatteryCheck<posX, posY, size, pin> name(reinterpret_cast<const __FlashStringHelper*>(name##BatteryText), nextMenu);

class BasicWalls : public Element {
public:
    BasicWalls(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t wallWidth, uint16_t color = WHITE, bool north = false, bool south = false, bool east = false, bool west = false)
        : _x(x), _y(y), _w(w), _h(h), _wallWidth(wallWidth), _color(color), _north(north), _south(south), _east(east), _west(west) {}

    void setX(uint8_t x) { _x = x; }
    void setY(uint8_t y) { _y = y; }
    void setW(uint8_t w) { _w = w; }
    void setH(uint8_t h) { _h = h; }
    void setColor(uint16_t color) { _color = color; }

    uint8_t getX() const { return _x; } 
    uint8_t getY() const { return _y; } 
    uint8_t getW() const { return _w; } 
    uint8_t getH() const { return _h; } 
    uint16_t getColor() const { return _color; }

    void draw(Display* parentDisplay) override {
        if (_north) parentDisplay->fill(DisPos(_x, _y), _w, _wallWidth, _color);
        if (_east) parentDisplay->fill(DisPos(_x + (_w - _wallWidth), _y), _wallWidth, _h, _color);
        if (_south) parentDisplay->fill(DisPos(_x, _y + (_h - _wallWidth)), _w, _wallWidth, _color);
        if (_west) parentDisplay->fill(DisPos(_x, _y), _wallWidth, _h, _color);
    }

protected:
    uint8_t _x;
    uint8_t _y;
    uint8_t _w;
    uint8_t _h;
    uint8_t _wallWidth;
    uint16_t _color;

    bool _north, _south, _east, _west;
};

typedef bool (*WallFunc)(void);

class FuncWalls : public BasicWalls {
public:
    FuncWalls(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t wallWidth, WallFunc north, WallFunc east, WallFunc south, WallFunc west, uint16_t color = WHITE)
        : BasicWalls(x, y, w, h, wallWidth, color), _north(north), _east(east), _south(south), _west(west) {}

    void setNorth(bool (*north)()) { _north = north; }
    void setEast(bool (*east)()) { _east = east; }
    void setSouth(bool (*south)()) { _south = south; }
    void setWest(bool (*west)()) { _west = west; }

    WallFunc getNorth() const { return _north; }
    WallFunc getEast() const { return _east; }
    WallFunc getSouth() const { return _south; }
    WallFunc getWest() const { return _west; }

    void draw(Display* parentDisplay) override {
        BasicWalls::_north = _north();
        BasicWalls::_east = _east();
        BasicWalls::_south = _south();
        BasicWalls::_west = _west();
        BasicWalls::draw(parentDisplay);
    }

protected:
    WallFunc _north;
    WallFunc _east;
    WallFunc _south;
    WallFunc _west;
};

// ----------------------------------------------------------------------------------------------------
// selectable classes
// ----------------------------------------------------------------------------------------------------

// class to inherit from when making a Selectable element
class Selectable {
public:
    Selectable() {}

    virtual void draw(bool selected, Display* displayParent) = 0;
    virtual void run(Display* displayParent, Menu* menuParent) = 0; // methods to override when you want to define the logic to happen
};

template <int8_t x, int8_t y, uint8_t size>
class SelectableConstText : public Selectable {
public:
    SelectableConstText(const __FlashStringHelper* text)
        : Selectable(), text(text) {}

    const __FlashStringHelper* text;

    void draw(bool selected, Display* displayParent) override {
        displayParent->print(text, DisPos(x, y), size, WHITE);
        if (selected)
            displayParent->fill(DisPos(x - 1, y - 1), flashStringLength(text) * size * _WIDTH_PER_TEXT_SIZE + 1, size * _HEIGHT_PER_TEXT_SIZE + 1, INVERT);
    }
};

template <int8_t x, int8_t y, uint8_t size>
class MenuSelectable : public SelectableConstText<x, y, size> {
public:
    MenuSelectable(const __FlashStringHelper* text, Menu* menu) 
        : SelectableConstText<x, y, size>(text), menu(menu) {}

    Menu* menu;

    void run(Display* displayParent, Menu* menuParent) override {
        displayParent->currMenu = menu;
    }
};

#define GEN_MENU_SELECTABLE(name, posX, posY, size, text, menuPtr) \
    const char name##Text[] PROGMEM = text; \
    MenuSelectable<posX, posY, size> name(reinterpret_cast<const __FlashStringHelper*>(name##Text), menuPtr);

template <int8_t x, int8_t y, uint8_t size>
class ScriptSelectable : public SelectableConstText<x, y, size> {
public:
    ScriptSelectable(const __FlashStringHelper* text, void (*script)(Display*, Menu*), Menu* scriptMenu = nullptr) 
        : SelectableConstText<x, y, size>(text), script(script), scriptMenu(scriptMenu) {}
    
    void (*script)(Display*, Menu*); // script to play on select (gets passed who it got called by)
    Menu* scriptMenu;

    void run(Display* displayParent, Menu* menuParent) override {
        if (scriptMenu != nullptr)
            displayParent->currMenu = scriptMenu;
        script(displayParent, menuParent);
    }
};

#define GEN_SCRIPT_SELECTABLE(name, posX, posY, size, text, script, scriptMenuPtr) \
    const char name##Text[] PROGMEM = text; \
    ScriptSelectable<posX, posY, size> name(reinterpret_cast<const __FlashStringHelper*>(name##Text), script, scriptMenuPtr);

#endif