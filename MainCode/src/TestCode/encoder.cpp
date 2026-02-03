#include <Arduino.h>
#include <debug.h>
#include <Encoders/encoders.h>

#include <config.h>

#define USE_encoder true
#if CAT(USE_, CURR_MAIN)

int main() {
    init();

    BEGIN_DEBUG(BAUDE_RATE);

    initEncoders();

    while (true)
    {
        DB_PRINTLN(getEncoderDeg());
        delay(1000);
    }
}

#endif