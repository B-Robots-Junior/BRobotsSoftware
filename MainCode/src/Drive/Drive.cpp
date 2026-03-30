#include <Drive/Drive.h>
#include <Arduino.h>

DualTB9051FTGMotorShield::DualTB9051FTGMotorShield()
{

  _PWM_1 = 6;
  _AIN1_1 = 38;
  _AIN2_1 = 36;


  _PWM_2 = 8;
  _AIN1_2 = 32;
  _AIN2_2 = 34;



  _PWM_3 = 7;
  _AIN1_3 = 24;
  _AIN2_3 = 25;

  _PWM_4 = 11;
  _AIN1_4 = 23; 
  _AIN2_4 = 22; 

  _flipM1 = false;
  _flipM2 = false;
  _flipM3 = false;
  _flipM4 = false;
}


DualTB9051FTGMotorShield::DualTB9051FTGMotorShield(
                             unsigned char PWM_1, unsigned char AIN1_1, unsigned char AIN2_1,
                             unsigned char PWM_2, unsigned char AIN1_2, unsigned char AIN2_2,
                             unsigned char PWM_3, unsigned char AIN1_3, unsigned char AIN2_3,
                             unsigned char PWM_4, unsigned char AIN1_4, unsigned char AIN2_4)
{
  _PWM_1 = PWM_1;
  _AIN1_1 = AIN1_1;
  _AIN2_1 = AIN2_1;

  _PWM_2 = PWM_2;
  _AIN1_2 = AIN1_2;
  _AIN2_2 = AIN2_2;

  _PWM_3 = PWM_3;
  _AIN1_3 = AIN1_3;
  _AIN2_3 = AIN2_3;

  _PWM_4 = PWM_4;
  _AIN1_4 = AIN1_4;
  _AIN2_4 = AIN2_4;

  _flipM1 = false;
  _flipM2 = false;
  _flipM3 = false;
  _flipM4 = false;
}

void DualTB9051FTGMotorShield::init()
{
  pinMode(_PWM_1, OUTPUT);
  pinMode(_AIN1_1, OUTPUT);
  pinMode(_AIN2_1, OUTPUT);

  pinMode(_PWM_2, OUTPUT);
  pinMode(_AIN1_2, OUTPUT);
  pinMode(_AIN2_2, OUTPUT);

  pinMode(_PWM_3, OUTPUT);
  pinMode(_AIN1_3, OUTPUT);
  pinMode(_AIN2_3, OUTPUT);

  pinMode(_PWM_4, OUTPUT);
  pinMode(_AIN1_4, OUTPUT);
  pinMode(_AIN2_4, OUTPUT);
}

void DualTB9051FTGMotorShield::setM1Speed(int speed)
{
  speed = constrain(speed, -255, 255);
  if (_flipM1 == true) { speed = speed * (-1); } 

  if (speed >= 0) {
    digitalWrite(_AIN1_1, HIGH);
    digitalWrite(_AIN2_1, LOW);
  } else {
    digitalWrite(_AIN1_1, LOW);
    digitalWrite(_AIN2_1, HIGH);
    speed = -speed;
  }
  analogWrite(_PWM_1, speed);
}


void DualTB9051FTGMotorShield::setM2Speed(int speed)
{
  speed = constrain(speed, -255, 255);
  if (_flipM2 == true) { speed = speed * (-1); }

  if (speed >= 0) {
    digitalWrite(_AIN1_2, HIGH);
    digitalWrite(_AIN2_2, LOW);
  } else {
    digitalWrite(_AIN1_2, LOW);
    digitalWrite(_AIN2_2, HIGH);
    speed = -speed;
  }
  analogWrite(_PWM_2, speed);
}


void DualTB9051FTGMotorShield::setM3Speed(int speed)
{
  speed = constrain(speed, -255, 255);
  if (_flipM3 == true) { speed = speed * (-1); }

  if (speed >= 0) {
    digitalWrite(_AIN1_3, LOW);
    digitalWrite(_AIN2_3, HIGH);
  } else {
    digitalWrite(_AIN1_3, HIGH);
    digitalWrite(_AIN2_3, LOW); 
    speed = -speed; 
  }
  analogWrite(_PWM_3, speed);
}


void DualTB9051FTGMotorShield::setM4Speed(int speed)
{
  speed = constrain(speed, -255, 255);
  if (_flipM4 == true) { speed = speed * (-1); }

  if (speed >= 0) {
    digitalWrite(_AIN1_4, LOW);
    digitalWrite(_AIN2_4, HIGH);
  } else {
    digitalWrite(_AIN1_4, HIGH);
    digitalWrite(_AIN2_4, LOW);
    speed = -speed;
  }
  analogWrite(_PWM_4, speed);
}


void DualTB9051FTGMotorShield::setSpeeds(int m1Speed, int m2Speed, int m3Speed, int m4Speed)
{
  setM1Speed(m1Speed);
  setM2Speed(m2Speed);
  setM3Speed(m3Speed);
  setM4Speed(m4Speed);
}

void DualTB9051FTGMotorShield::flipM1(boolean flip) { _flipM1 = flip; }
void DualTB9051FTGMotorShield::flipM2(boolean flip) { _flipM2 = flip; }
void DualTB9051FTGMotorShield::flipM3(boolean flip) { _flipM3 = flip; }
void DualTB9051FTGMotorShield::flipM4(boolean flip) { _flipM4 = flip; }


void DualTB9051FTGMotorShield::enableDrivers()
{
  digitalWrite(27, HIGH);
}

void DualTB9051FTGMotorShield::disableDrivers()
{
  digitalWrite(27, LOW);
}