#ifndef CONFIG_H
#define CONFIG_H

#define PRIM_CAT(a, b) a##b
#define CAT(a, b) PRIM_CAT(a, b)

#define BAUDE_RATE 115200

#define CURR_MAIN drive

#define P_OFFSET_EEPROM 0
#define I_OFFSET_EEPROM 2
#define D_OFFSET_EEPROM 4

#endif