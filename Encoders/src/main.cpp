#include <Arduino.h>
#include <PinChangeInterrupt.h>

#define M1_ENC_A 7
#define M1_ENC_B 6

#define M2_ENC_A 9
#define M2_ENC_B 8

#define M3_ENC_A 3
#define M3_ENC_B 2

#define M4_ENC_A 5
#define M4_ENC_B 4

#define REQUEST_M1_MM 1
#define REQUEST_M2_MM 2
#define REQUEST_M3_MM 3
#define REQUEST_M4_MM 4

#define REQUEST_AVR 5

#define RESET_ALL 'R'
#define SET_AVR 'S'

#define WHEEL_DIAMETER_MM 80

// with a 75:1 metal gearbox and an integrated quadrature encoder that provides a resolution of 11 counts per revolution of the motor shaft,
// which corresponds to 823.1 counts per revolution of the gearbox's output shaft.

#define COUNTS_PER_ROT 823.1
#define MM_PER_COUNT ((1.0 / COUNTS_PER_ROT) * PI * WHEEL_DIAMETER_MM)

double m1Dist = 0;
double m2Dist = 0;
double m3Dist = 0;
double m4Dist = 0;

#define DEF_MOTOR_INT(n, mul) \
    void m##n##Int() { \
        m##n##Dist += digitalRead(M##n##_ENC_B) ? -MM_PER_COUNT * mul : MM_PER_COUNT * mul; \
    } \

DEF_MOTOR_INT(1,  1);
DEF_MOTOR_INT(2, -1);
DEF_MOTOR_INT(3,  1);
DEF_MOTOR_INT(4, -1);

#define ATTACH_MOTOR_INT(n) \
    attachPCINT(digitalPinToPCINT(M##n##_ENC_A), m##n##Int, FALLING);

#define M_PIN_MODE(n) \
    pinMode(M##n##_ENC_A, INPUT_PULLUP); \
    pinMode(M##n##_ENC_B, INPUT_PULLUP);

int main() {
    init();
    Serial.begin(9600);

    M_PIN_MODE(1);
    M_PIN_MODE(2);
    M_PIN_MODE(3);
    M_PIN_MODE(4);

    ATTACH_MOTOR_INT(1);
    ATTACH_MOTOR_INT(2);
    ATTACH_MOTOR_INT(3);
    ATTACH_MOTOR_INT(4);

    double* motorDists[] = {&m1Dist, &m2Dist, &m3Dist, &m4Dist};

    while (true) {
        if (Serial.available() != 0) {
            uint8_t req = Serial.read();
            if (req >= REQUEST_M1_MM && req <= REQUEST_M4_MM) {
                Serial.write((uint8_t*)motorDists[req-1], sizeof(double));
            }
            else if (req == REQUEST_AVR) {
                double avr = (m1Dist + m2Dist + m3Dist + m4Dist) / 4.0;
                Serial.write((uint8_t*)&avr, sizeof(double));
            }
            else if (req == RESET_ALL) {
                m1Dist = 0;
                m2Dist = 0;
                m3Dist = 0;
                m4Dist = 0;
            }
            else if (req == SET_AVR) {
                double avr = 0;
                Serial.readBytes((uint8_t*)&avr, sizeof(double));
                m1Dist = avr;
                m2Dist = avr;
                m3Dist = avr;
                m4Dist = avr;
            }
        }
    }
}