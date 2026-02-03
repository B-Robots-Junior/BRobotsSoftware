#include <encoders.h>

// T5 pin -> PL2 pin -> D47 pin

volatile uint32_t encoder_overflows = 0;

void initEncoders() {
    TCCR5A = 0x00;
    TCCR5B = 0b00000111;
    TIMSK5 = 0b00000001;
}

float getEncoderDeg() {
    return getEncoderValue() * DEG_PER_COUNT;
}

float getEncoderValueMM() {
    return getEncoderValue() * MM_PER_COUNT;
}

uint64_t getEncoderValue() {
    uint8_t hcnt = TCNT5H;
    return (encoder_overflows << 16) | (hcnt << 8) | TCNT5L;
}

ISR(TIMER5_OVF_vect) { // once the timer overflows increment the encoder_overflows counter
    encoder_overflows++;
}