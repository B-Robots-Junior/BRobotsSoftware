#ifndef PID_H
#define PID_H

#include <Arduino.h>

typedef float (*PIDFormula)(float error, float integral, float derivative, float kp, float ki, float kd);

class PID {
public:
    PID();

    void reset();

    float calculate(float error, float Kp, float Ki, float Kd, float integralLimit);

private:
    unsigned long _lastTime;
    float _prevError;
    float _integral;

    PIDFormula _pidCalculation;
};

#endif