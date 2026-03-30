#ifndef CONFIG_H
#define CONFIG_H

#define PRIM_CAT(a, b) a##b
#define CAT(a, b) PRIM_CAT(a, b)

#define BAUDE_RATE 115200

#define CURR_MAIN main_func

// eeprom settings
#define P_OFFSET_EEPROM 0 // 2 bytes float
#define I_OFFSET_EEPROM 2 // 2 bytes float
#define D_OFFSET_EEPROM 4 // 2 bytes float

#define SPECTROMETER_OFFSET_EEPROM 6 // (12 channels uint16_t (24 bytes)) * 5 colors = 120 bytes
#define RGBC_SENSOR_OFFSET_EEPROM 126 // (4 channels uint16_t (8 bytes)) * 5 colors = 40 bytes
#define REF_SENSOR_OFFSET_EEPROM 166 // (1 channel uint16_t (2 bytes)) * 5 colors = 10 bytes

#endif