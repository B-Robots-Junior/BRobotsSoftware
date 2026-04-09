#include <Encoders/encoders.h>
#include <util/atomic.h>

// T5 pin -> PL2 pin -> D47 pin

volatile uint32_t encoder_overflows = 0;

bool initEncoders() {
    TCCR5A = 0x00;
    TCCR5B = 0b00000111;
    TIMSK5 = 0b00000001;
    return true;
}

float getEncoderDeg() {
    return getEncoderValue() * DEG_PER_COUNT;
}

float getEncoderValueMM() {
    return getEncoderValue() * MM_PER_COUNT;
}

uint64_t getEncoderValue() {
    return 0;

    uint16_t tcnt;
    uint32_t overflows;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        tcnt = TCNT5;
        overflows = encoder_overflows;

        if ((TIFR5 & (1 << TOV5)) && (tcnt < 65535)) {
            overflows++;
        }
    }

    return ((uint64_t)overflows << 16) | tcnt;
}

ISR(TIMER5_OVF_vect) { // once the timer overflows increment the encoder_overflows counter
    encoder_overflows++;
}