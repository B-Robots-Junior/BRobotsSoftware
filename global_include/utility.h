#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <Arduino.h>
#include <MemoryFree.h>

template <typename T>
constexpr T&& move(T& t) noexcept {
    return static_cast<T&&>(t);
}

inline double wrap(double value, double lowerBound, double upperBound) {
    double rangeSize = upperBound - lowerBound;

    if (rangeSize <= 0) return lowerBound;
    double wrappedValue = fmod((value - lowerBound), rangeSize);
    if (wrappedValue < 0) {
        wrappedValue += rangeSize;
    }

    return wrappedValue + lowerBound;
}

inline float angleDiffDEG(float angle1, float angle2) {
    float diff = fmod(angle1 - angle2 + 180.0f, 360.0f);
    if (diff < 0)
        diff += 360.0f;
    return diff - 180.0f;
}

inline size_t flashStringLength(const __FlashStringHelper* fstr) {
  return strlen_P(reinterpret_cast<PGM_P>(fstr));
}

inline float wrap360(float angle) {
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;
  return angle;
}

inline float wrap180(float angle) {
  while (angle <= -180) angle += 360;
  while (angle > 180) angle -= 360;
  return angle;
}

inline uint8_t voltageToPercent(float volt) {
  return 123 - 123 / powf((1 + powf((volt / 2) / 3.7, 80)), 0.165);
}

#endif