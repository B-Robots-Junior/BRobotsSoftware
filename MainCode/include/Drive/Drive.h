#pragma once

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || \
    defined(__AVR_ATmega328PB__) || defined (__AVR_ATmega32U4__)
  #define DUALTB9051FTGMOTORSHIELD_TIMER1_AVAILABLE
#endif

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <VL6180X.h> 
#include <VL53L0X.h>
#include <debug.h>
#include <pos.h>
//#include <encoders.h>


class DualTB9051FTGMotorShield
{
  public:

    DualTB9051FTGMotorShield();
    DualTB9051FTGMotorShield(unsigned char PWM_1,
                             unsigned char AIN1_1,
                             unsigned char AIN2_1,
                             unsigned char PWM_2 ,
                             unsigned char AIN1_2,
                             unsigned char AIN2_2,
                             unsigned char PWM_3,
                             unsigned char AIN1_3,
                             unsigned char AIN2_3,
                             unsigned char PWM_4,
                             unsigned char AIN1_4 ,
                             unsigned char AIN2_4);
   


    // PUBLIC METHODS
    float getcurrentangle(void);
    void init(); // Initialize pins and timer1 if applicable.
    void setM1Speed(int speed); // Set speed for M1.
    void setM2Speed(int speed); // Set speed for M2.
    void setM3Speed(int speed); // Set speed for M3.
    void setM4Speed(int speed); // Set speed for M4.
    void setSpeeds(int m1Speed, int m2Speed); // Set speed for M1 and M2.
    void setSpeeds(int m1Speed, int m2Speed, int m3Speed, int m4Speed); // Set speed for all motors.
    unsigned char getM1Fault(); // Get fault reading from M1.
    unsigned char getM2Fault(); // Get fault reading from M2.
    unsigned char getM3Fault(); // Get fault reading from M3.
    unsigned char getM4Fault(); // Get fault reading from M4.
    void flipM1(boolean flip); // Flip the direction of the speed for M1.
    void flipM2(boolean flip); // Flip the direction of the speed for M2.
    void flipM3(boolean flip); // Flip the direction of the speed for M3.
    void flipM4(boolean flip); // Flip the direction of the speed for M4.
    void enableM1Driver(); // Enable the driver for M1.
    void enableM2Driver(); // Enable the driver for M2.
    void enableM3Driver(); // Enable the driver for M3.
    void enableM4Driver(); // Enable the driver for M4.
    void enableDrivers(); // Enables the drivers for all motors.
    void disableM1Driver(); // Disable the driver for M1.
    void disableM2Driver(); // Disable the driver for M2.
    void disableM3Driver(); // Disable the driver for M3.
    void disableM4Driver(); // Disable the driver for M4.
    void disableDrivers(); // Disable the drivers for all motors.
    unsigned int getM1CurrentMilliamps(); // Get current reading for M1.
    unsigned int getM2CurrentMilliamps(); // Get current reading for M2.
    unsigned int getM3CurrentMilliamps(); // Get current reading for M3.
    unsigned int getM4CurrentMilliamps(); // Get current reading for M4.
    int Turnrobot(float targetAngle, uint64_t startTime, float minMultiplier = 0.8, float tolerance = 0.2);//Turn the robot for a wished angle.
    void calibrateGyro(float);//Calibrate the gyro sensor. 
    void perfectedDriving(float);// Drive after a wished angle.
    float position(void); // Calculating the current position in angle and return it.
    void VL53Lox();
    float Getfronttofvalue();
    float Getbacktofvalue();
    int GetFrontTof();
    int driveAlong(int targetBackDist, int targetFrontDist, float targetDist, float targetAngle, double targetEncoderDist = -1, float speedMul = 1);
    int driveAlongEncoders(double targetEncoderVal, float targetDist, float targetAngle, float speedMul = 1);
    //int driveAlong(int targetDist, int (*tofFunc)(void), bool distShouldIncr, float targetSideDist, float targetAngle, float speedMul = 1);
    void keepCentered(float speedMul = 1); 
    void keepHeading(float targetAngle, float speedMul = 1);
    void keepCenteredgyro(float angle, float speedMul = 1);
    int driveBezier(Pos<float> p0, Pos<float> p1, Pos<float> pa, int baseSpeed, int turnSpeed, uint32_t startTime, uint32_t duration);
    int driveSCurve(int speed, uint16_t radius, uint32_t startTime, uint32_t duration, int direc); // direc = 1 for to left back, direc = -1 for to right back and radius in mm
    void resetTimeSensativeVars();

  private:
    unsigned char _M1PWM;
    static const unsigned char _M1PWM_TIMER1_PIN = 9;
    unsigned char _M2PWM;
    static const unsigned char _M2PWM_TIMER1_PIN = 10;
    unsigned char _PWM_1;
    unsigned char _AIN1_1;
    unsigned char _AIN2_1;
    unsigned char _PWM_2 ;
    unsigned char _AIN1_2;
    unsigned char _AIN2_2;
    unsigned char _PWM_3;
    unsigned char _AIN1_3;
    unsigned char _AIN2_3;
    unsigned char _PWM_4;
    unsigned char _AIN1_4;
    unsigned char _AIN2_4;
    boolean _flipM1;
    boolean _flipM2;
    boolean _flipM3;
    boolean _flipM4;
    float _gyroNullpunktX ;
    float _gyroNullpunktY ;
    float _gyroNullpunktZ ;

    bool _driveAlongFailed = false;
    bool _frontFailed = false;
    bool _backFailed = false;

    uint32_t lastTime = millis();
    float lastError = 0;
    float integral = 0;

    uint32_t lastTime1 = millis();
    float lastError1 = 0;
    float integral1 = 0;

    float _startBezierAngle = -1;
    uint32_t _lastBezierTime = 0;

};

