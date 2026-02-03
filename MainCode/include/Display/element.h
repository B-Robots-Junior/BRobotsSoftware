#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <Arduino.h>
#include <pos.h>
#include <constants.h>
#include <Display/display.h>
#include <Display/menu.h>
#include "devices.h"

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

template <int8_t x, int8_t y, uint8_t size>
class WallScetch : public TileElement<x, y, size> {
public:
    WallScetch(bool (*front)(), bool (*right)(), bool (*left)(), bool (*back)())
        : TileElement<x, y, size>(), front(front), right(right), left(left), back(back) {}

    bool (*front)();
    bool (*right)();
    bool (*left)();
    bool (*back)();

    void draw(Display* parentDisplay) override {
        TileElement<x, y, size>::data.north = front();
        TileElement<x, y, size>::data.east = right();
        TileElement<x, y, size>::data.south = back();
        TileElement<x, y, size>::data.west = left();
        TileElement<x, y, size>::draw(parentDisplay);
    }
};

#define GEN_WALL_SCETCH(name, posX, posY, size, front, right, left, back) \
    WallScetch<posX, posY, size> name(front, right, left, back);

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