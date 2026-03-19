#ifndef _EEPROM_UTIL_H_
#define _EEPROM_UTIL_H_

#include <EEPROM.h>

// ----------------------------------------------------------------------------------------------------
// EEPROM utility
// ----------------------------------------------------------------------------------------------------

// saves a variable to the EEPROM (the next free index is NOT index + 1 instead it is index + sizeof(T))
// to save a varibale us the syntax: saveToEEPROM<[variable type]>([index], [value])
template <typename T>
void saveToEEPROM(int index, T value) { 
    for (size_t i = 0; i < sizeof(value); i++)
        EEPROM.write(index + i, ((uint8_t*)&value)[i]);
}

// reads a value from the EEPROM at indey (the next index is NOT index + 1 instead it is index + sizeof(T))
// syntax veToEEPROM<[variable type]>([index])
template <typename T>
T readFromEEPROM(int index) {
    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++)
        ((uint8_t*)&value)[i] = EEPROM.read(index + i);
    return value;
}

#endif