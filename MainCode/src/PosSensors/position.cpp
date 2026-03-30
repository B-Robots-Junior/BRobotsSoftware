#include <Arduino.h>
#include <Wire.h>
#include <VL6180X.h> 
#include <PosSensors/ToF.h> 
#include <math.h>
#include <Adafruit_Sensor.h>
#include <utility.h>
#define _POSITION_H
#include <PosSensors/position.h>
#include <PosSensors/gyroSensor.h>
#include <constants.h>

float nullpunkt_X = 0.0;
float nullpunkt_Y = 0.0;
float nullpunkt_Z = 0.0;

// right and left were swaped
// after that back left and back right where swaped
ShortToF TofLF(43, 0x31);    // was: XSHUT_pin7,Sensor7_newAddress
ShortToF TofLB(40, 0x42);    // was: XSHUT_pin4,Sensor4_newAddress
ShortToF TofRF(37, 0x33);    // was: XSHUT_pin6,Sensor6_newAddress
ShortToF TofRB(35, 0x34);    // was: XSHUT_pin5,Sensor5_newAddress
NewLongToF Back(39, 0x35);   // was: XSHUT_pin3,Sensor3_newAddress); // 34
NewLongToF frontTop(41, 0x36);
// LongToF frontBottom(); //! add this when pinout is given

GyroSensor gyro;

#define INIT_TOF(tof) \
    bool tof##Worked = !tof.init_ToF(); \
    if (!tof##Worked) { \
        DB_PRINT_MUL((SET_RED)(F(#tof))(F(" failed to init!"))(RESET_COLOR)('\n')); \
    } \
    worked = worked && tof##Worked;


bool initTofs(void)
{
    bool worked = true;
    Wire.begin();

    DB_PRINTLN(F("Starte TofLB..."));
    INIT_TOF(TofLB);
    DB_PRINTLN(F("TofLF ok"));

    DB_PRINTLN(F("Starte TofLF..."));
    INIT_TOF(TofLF);
    DB_PRINTLN(F("TofLF OK."));

    DB_PRINTLN(F("Starte TofRB..."));
    INIT_TOF(TofRB);
    DB_PRINTLN(F("TofRB OK."));

    DB_PRINTLN(F("Starte TofRF..."));
    INIT_TOF(TofRF);
    DB_PRINTLN(F("TofRF OK."));

    DB_PRINTLN(F("Starte frontTop..."));
    INIT_TOF(frontTop);
    DB_PRINTLN(F("frontTop OK."));

    DB_PRINTLN(F("Starte Back..."));
    INIT_TOF(Back);
    DB_PRINTLN(F("Back OK."));

    return worked;
}

void updateTofs(void)
{
    TofLF.read();
    TofLB.read();
    TofRF.read();
    TofRB.read();
    Back.read();
    frontTop.read();
}

bool initgyro(void)
{
    if (!gyro.begin())
    {
        Serial.println(F("Gyro could not be initialized"));
        return false;
    }
    return true;
}

double ReadGyroyaw()
{
    return gyro.getYaw();
}

double ReadGyropitch()
{
    return gyro.getPitch();
}

double ReadGyroroll()
{
    return gyro.getRoll();
}

double radtograd (double Rad)
{
    return Rad * (180.0 / PI);
}

double calculateposition(void)
{
    double RightFront = TofRF.read();
    double RightBack = TofRB.read();

    double LeftFront = TofLF.read();
    double LeftBack = TofLB.read();

    bool rightValid = RightFront >= 0 && RightFront < 255 && RightBack >= 0 && RightFront < 255;
    bool leftValid = LeftFront >= 0 && LeftFront < 255 && LeftBack >= 0 && LeftFront < 255;

    double AngleRightRAD;  
    double AngleLeftRAD;

    AngleRightRAD = atan((RightFront-RightBack)/135.0);
    AngleLeftRAD = atan((LeftBack-LeftFront)/135.0);
  
    double AngleRightGRD = (AngleRightRAD * (180.0/M_PI));
    double AngleLeftGRD = (AngleLeftRAD * (180.0/M_PI) + 14.0);

    double Angle = 0;
    if (!rightValid) {
        if (!leftValid)
            Angle = 0;
        else
            Angle = AngleLeftGRD;
    }
    else {
        if (!leftValid)
            Angle = AngleRightGRD;
        else
            Angle = (AngleRightGRD+AngleLeftGRD)/2.0;
    }

    return Angle;
}

// int getFrontDistance() //! CHANGED TO FRONTTOP SO I HAVE TO REPLACE LESS CODE
// {
//     return frontTop.read() + TOF_FRONT_ADJ;
// }

int getBackDistance()
{
    return Back.read() + TOF_BACK_ADJ;
}

int getLFDistance()
{
    return TofLF.read() + TOF_LF_ADJ;
}

int getLBDistance()
{
    return TofLB.read() + TOF_LB_ADJ;
}

int getRFDistance()
{
    return TofRF.read() + TOF_RF_ADJ;
}

int getRBDistance()
{
    return TofRB.read() + TOF_RB_ADJ;
}

int getFrontTopDistance() {
    return frontTop.read() + TOF_FRONT_TOP_ADJ;
}

float getFrontAngle() {
    return atan2(FRONT_HEIGHT_DELTA_MM, getFrontTopDistance() - getFrontDistance()) * RAD_TO_DEG;
}

float getRightDistance() {
    int frontDist = getRFDistance();
    int backDist = getRBDistance();
    bool frontValid = frontDist >= 0 && frontDist < 255;
    bool backValid = backDist >= 0 && backDist < 255;
    if (!frontValid) {
        if (!backValid)
            return 255;
        return backDist;
    }
    if (!backValid)
        return frontDist;
    return (frontDist + backDist) / 2.0;
}

float getLeftDistance() {
    int frontDist = getLFDistance();
    int backDist = getLBDistance();
    bool frontValid = frontDist >= 0 && frontDist < 255;
    bool backValid = backDist >= 0 && backDist < 255;
    if (!frontValid) {
        if (!backValid)
            return 255;
        return backDist;
    }
    if (!backValid)
        return frontDist;
    return (frontDist + backDist) / 2.0;
}

bool wallFront() {
    return getFrontTopDistance() <= (300 - ROBOT_LENGHT_MM + WALL_DETECT_EXTRA);
}

bool wallBack() {
    return getBackDistance() <= (300 - ROBOT_LENGHT_MM + WALL_DETECT_EXTRA);
}

bool wallRight() {
    return getRightDistance() <= (300 - ROBOT_WIDTH_MM + WALL_DETECT_EXTRA);
}

bool wallLeft() {
    return getLeftDistance() <= (300 - ROBOT_WIDTH_MM + WALL_DETECT_EXTRA);
}