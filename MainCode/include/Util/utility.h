#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>

double wrap(double value, double lowerBound, double upperBound); // [lowerBound, upperBound[
float wrap360(float angle);
float wrap180(float angle);
float angleDiffDEG(float angle1, float angle2); // the signed difference of two angles

inline size_t flashStringLength(const __FlashStringHelper* fstr) {
  return strlen_P(reinterpret_cast<PGM_P>(fstr));
}

#endif // UTILITY_H