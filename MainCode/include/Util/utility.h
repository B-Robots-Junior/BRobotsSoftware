#ifndef UTILITY_H
#define UTILITY_H

double wrap(double value, double lowerBound, double upperBound); // [lowerBound, upperBound[
float wrap360(float angle);
float wrap180(float angle);
float angleDiffDEG(float angle1, float angle2); // the signed difference of two angles

#endif // UTILITY_H