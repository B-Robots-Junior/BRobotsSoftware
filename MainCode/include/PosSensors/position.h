#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <VL6180X.h>  
#include <Adafruit_BNO055.h>
#include <constants.h>
#include <PosSensors/gyroSensor.h>
#include <PosSensors/ToF.h>

// TOF
#define TOF_TIMEOUT_VALUE 800

#ifdef _POSIION_H
    //Adafruit_BNO055 bno = Adafruit_BNO055(55);
    //ShortToF TofLF(XSHUT_pin4,Sensor4_newAddress);
    //ShortToF TofLB(XSHUT_pin6,Sensor6_newAddress);
    //ShortToF TofRF(XSHUT_pin6,Sensor6_newAddress);
    //ShortToF TofRB(XSHUT_pin5,Sensor5_newAddress);
    //ShortToF Back(XSHUT_pin3,Sensor3_newAddress);
    //MiddleToF front(XSHUT_pin1,Sensor1_newAddress);
    
#else 
   //extern Adafruit_BNO055 bno; 
   //extern ShortToF TofRF(XSHUT_pin6,Sensor6_newAddress);
   //extern ShortToF TofRB(XSHUT_pin5,Sensor5_newAddress);
#endif

extern GyroSensor gyro;

bool getTofLFValid();
bool getTofLBValid();
bool getTofRFValid();
bool getTofRBValid();
bool getTofFBLValid();
bool getTofBValid();
bool getTofFTValid();
bool getTofFBRValid();

// function for calculating the angle of the robot using rear tofs
double calculateposition (void);
// convert radiant to grad
double radtograd (void);
//convert grad to radiant
double gradtorad (void);
bool initTofs(void);
void updateTofs(void);
bool initgyro(void);
double ReadGyroyaw();
double ReadGyropitch();
double ReadGyroroll();
int getFrontDistance(); 
int getBackDistance();
int getLFDistance();
int getLBDistance();
int getRFDistance();
int getRBDistance();
int getFrontTopDistance();
int getFrontBottomLongDistance();
int getFrontBottomShortDistance();
float getFrontAngle();
float getRightDistance();
float getLeftDistance();

bool wallFront();
bool wallBack();
bool wallRight();
bool wallLeft();