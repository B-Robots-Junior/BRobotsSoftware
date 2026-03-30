#ifndef COLOR_TYPE_H
#define COLOR_TYPE_H

#include <Arduino.h>

enum class ColorType : int8_t {
    Invalid = -1,
    Normal = 0,
    Blue = 1,
    Black = 2,
    Checkpoint = 3,
    Red = 4
};

extern const char* colorTypeToAbrv[5];
extern const char* colorTypeToName[5];

#endif