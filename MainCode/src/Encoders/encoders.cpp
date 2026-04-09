#include <Encoders/encoders.h>
#include <util/atomic.h>

volatile uint32_t encoder_overflows = 0;

bool initEncoders() {
    // 1. Initialize the pin (Crucial for open-collector encoders)
    pinMode(47, INPUT_PULLUP);

    // 2. Stop the timer while we configure it
    TCCR5A = 0x00;
    TCCR5B = 0x00;
    
    // 3. Clear counters to start from a clean slate
    TCNT5 = 0;
    encoder_overflows = 0;

    // 4. Clear any pending overflow flags that might exist from boot
    TIFR5 |= (1 << TOV5);
    
    // 5. Enable the interrupt and start the timer (Rising edge on T5)
    TIMSK5 = 0b00000001;
    TCCR5B = 0b00000111; 
    
    return true;
}

// Note: If values exceed ~16.7 million counts, these floats will lose precision.
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

        // FIX: Check against a low threshold to prevent massive data spikes
        if ((TIFR5 & (1 << TOV5)) && (tcnt < 255)) {
            overflows++;
        }
    }

    return ((uint64_t)overflows << 16) | tcnt;
}

ISR(TIMER5_OVF_vect) { 
    encoder_overflows++;
}