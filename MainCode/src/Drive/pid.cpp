#include <Drive/pid.h>

PID::PID() {
    reset();

    _pidCalculation = [](float error, float integral, float derivative, float kp, float ki, float kd) -> float {

        return (kp * error) + (ki * integral) + (kd * derivative);
    };
}

void PID::reset() {
    _lastTime = 0;
    _prevError = 0;
    _integral = 0;
}

float PID::calculate(float error, float Kp, float Ki, float Kd, float integralLimit) {
    unsigned long now = millis();
    

    if (_lastTime == 0) {
        _lastTime = now;
        _prevError = error;
        return 0;
    }


    float deltaTime = (now - _lastTime) / 1000.0;
    

    if (deltaTime > 0.5 || deltaTime <= 0.0) {
         deltaTime = 0.01; 
    }
    _lastTime = now;


    _integral += error * deltaTime;
    _integral = constrain(_integral, -integralLimit, integralLimit);

    float derivative = (error - _prevError) / deltaTime;
    _prevError = error;


    float output = _pidCalculation(error, _integral, derivative, Kp, Ki, Kd);

    return output;
}