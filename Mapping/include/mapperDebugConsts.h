#ifndef _MAPPER_DEBUG_CONSTS_H_
#define _MAPPER_DEBUG_CONSTS_H_

#include <Arduino.h>

static const char direcChar[4] = {'^', '>', 'v', '<'};

static const String type_middles[6] = {
    "\033[100m \033[0m",        // undiscovered (gray)
    "\033[100m \033[0m",        // normal (gray)
    "\033[44m \033[0m",         // blue tile (blue)
    "\033[90m\033[40mB\033[0m", // black tile (gray B with black background)
    "\033[43m \033[0m",         // Ramp tile (yellow)
    "\033[107m \033[0m"         // checkpoint tile (bright White)
};

static const String type_atering[6] = {
    "\033[100m",        // undiscovered (gray)
    "\033[100m",        // normal (gray)
    "\033[44m",         // blue tile (blue)
    "\033[90m\033[40m", // black tile (gray Arrow with black background)
    "\033[43m",         // Ramp tile (yellow)
    "\033[107m"         // checkpoint tile (bright White)
};

#endif