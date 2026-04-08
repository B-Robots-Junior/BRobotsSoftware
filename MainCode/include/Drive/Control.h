#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include <Drive/Drive.h>
#include <Drive/Pid.h>
#include <pos.h>  
#include <PosSensors/position.h>
#include <PosSensors/gyroSensor.h>
#include <Encoders/encoders.h>
#include <constants.h>

#define TURN_KP 4.0
#define TURN_KI 1.2
#define TURN_KD 1
#define TURN_INT_LIMIT 50.0
#define MAX_SPEED_TURN 175
#define MIN_SPEED_TURN 30

#define CENTER_KP 6.0
#define CENTER_KI 0.002
#define CENTER_KD 2.0
#define CENTER_INT_LIMIT 5.0
#define TARGET_CENTER_WALL_DIST 85.0 // !! in mm 

#define HEADING_KP 3.0
#define HEADING_KI 0.003
#define HEADING_KD 9.0
#define HEADING_INT_LIMIT 5.0

#define GYRO_KP 5.0
#define GYRO_KI 0.002
#define GYRO_KD 2.0
#define GYRO_INT_LIMIT 5.0

#define BASE_SPEED_DRIVE 130
#define MAX_CORRECTION_DRIVE 70


class Control {
public:
    Control(DualTB9051FTGMotorShield &motors);

    void resetPIDs();

    int turnRobot(float targetAngle, uint32_t startTime, float tolerance = 1.0);

    void keepCentered(float speedMul);
    void keepHeading(float targetDistance, float speedMul);
    void keepCenteredgyro(float angle, float speedMul);

    int driveAlong(int targetBackDist, int targetFrontDist, float targetDist, float target_angle, int64_t& lastEncoderDist, int64_t& trueEncoderDist, int64_t targetEncoderDist, float speedMul, int wallStoppingDist = FRONT_WALL_DIST_MM);
    void uncondDriveAlong(float targetDist, float target_angle, float speedMul); // drive along with no exit conditions
    int driveAlongEncoders(double targetEncoderVal, float targetDist, float targetAngle, float speedMul);
    
    int driveBezier(Pos<float> p0, Pos<float> p1, Pos<float> pa, int baseSpeed, int turnSpeed, uint32_t startTime, uint32_t duration);
    int driveSCurve(int speed, uint16_t radius, uint32_t startTime, uint32_t duration, int direc);

private:
    DualTB9051FTGMotorShield &_motors;
    
    PID _pidTurn;
    PID _pidDrive;   
    PID _pidHeading; 
    PID _pidGyro;     

    
    bool _turnActive; 
    
    bool _driveAlongFailed = false;
    bool _frontFailed = false;
    bool _backFailed = false;

    float _startBezierAngle = -1.0;
};

#endif