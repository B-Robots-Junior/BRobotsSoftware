#include <config.h>
#include <debug.h>
#include <Encoders/encoders.h>
#include <devices.h>

#define USE_encoderTest true
#if CAT(USE_, CURR_MAIN)
#undef USE_encoderTest

void rgbcSensorOnEnter() {}
void rgbcSensorOnExit() {}

int main() {
    init();
    sei();
    BEGIN_DEBUG(BAUDE_RATE);
    
    initEncoders();

    Devices::motors.init();

    Devices::motors.setSpeeds(100, 100, 100, 100);

    millis();

    uint64_t encoderCount = getEncoderValue();
    while(1) {
        millis();
        delay(20);
        DB_PRINT_MUL((CLEAR_SCREEN_AND_HOME)(F("Encoders: "))((unsigned long)getEncoderValue())(F(", Diff: "))(((long)getEncoderValue()) - ((long)encoderCount)));
        if (((long)getEncoderValue()) - ((long)encoderCount) < 0)
            ERROR(F("Encoder diff less than 0!"));
        encoderCount = getEncoderValue();
    }
}

#endif