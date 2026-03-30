#include <Util/utility.h>
#include <Arduino.h>

double wrap(double value, double lowerBound, double upperBound) {
    double rangeSize = upperBound - lowerBound;

    if (rangeSize <= 0) return lowerBound;
    double wrappedValue = fmod((value - lowerBound), rangeSize);
    if (wrappedValue < 0) {
        wrappedValue += rangeSize;
    }

    return wrappedValue + lowerBound;
}

float wrap360(float angle) {
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;
  return angle;
}

float wrap180(float angle) {
  while (angle <= -180) angle += 360;
  while (angle > 180) angle -= 360;
  return angle;
}

float angleDiffDEG(float angle1, float angle2) {
    float diff = fmod(angle1 - angle2 + 180.0f, 360.0f);
    if (diff < 0)
        diff += 360.0f;
    return diff - 180.0f;
}