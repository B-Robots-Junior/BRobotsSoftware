#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#include <Arduino.h>

#define WHEEL_DIAMETER_MM 80

// with a 75:1 metal gearbox and an integrated quadrature encoder that provides a resolution of 11 counts per revolution of the motor shaft,
// which corresponds to 823.1 counts per revolution of the gearbox's output shaft.

#define COUNTS_PER_ROT 823.1
#define DEG_PER_COUNT (360.0 / COUNTS_PER_ROT)
#define MM_PER_COUNT ((1.0 / COUNTS_PER_ROT) * PI * WHEEL_DIAMETER_MM)


extern volatile uint32_t encoder_overflows;

void initEncoders();
float getEncoderDeg();
float getEncoderValueMM();
uint64_t getEncoderValue();

#endif