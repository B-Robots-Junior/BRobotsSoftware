#ifndef TC_H
#define TC_H

#include <Arduino.h>
#include <Util/eepromUtil.h>

#include <config.h>

#define TC_INVALID 0
#define TC_TILE 1
#define TC_WALL 2
#define TC_VICTIM 3

struct tc_element_t {
    int8_t x;
    int8_t y;
    uint8_t type;
    uint16_t data;
};

inline void resetTC() {
    saveToEEPROM<uint16_t>(TC_OFFSET_EEPROM, 0);
}

inline uint16_t getNumElements() {
    return readFromEEPROM<uint16_t>(TC_OFFSET_EEPROM);
}

inline void addElement(tc_element_t element) {
    uint16_t numElements = getNumElements();
    saveToEEPROM<tc_element_t>(TC_OFFSET_EEPROM + 2 + sizeof(tc_element_t) * numElements, element);
    saveToEEPROM<uint16_t>(TC_OFFSET_EEPROM, numElements + 1);
}

inline tc_element_t getElement(uint16_t index) {
    if (index >= getNumElements())
        return tc_element_t { .x = 0, .y = 0, .type = TC_INVALID, .data = 0 };
    return readFromEEPROM<tc_element_t>(TC_OFFSET_EEPROM + 2 + sizeof(tc_element_t) * index);
}

#endif