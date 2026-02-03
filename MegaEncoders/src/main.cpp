#include <Arduino.h>
#include <debug.h>

volatile uint32_t overflows;

// PL2 -> 47 (T5)

int main() {
    init();

    BEGIN_DEBUG(9600);

    sei(); // general interrupt enable

    TCCR5A = 0x00;
    TCCR5B = 0b00000111;
    TIMSK5 = 0b00000001;

    while (1) {
        delay(1000);
        uint8_t hcnt = TCNT5H;
        uint64_t cnt = (overflows << 16) | (hcnt << 8) | TCNT5L;
        VAR_PRINTLN((long)cnt);
    } 
}

ISR(TIMER5_OVF_vect) {
    overflows++;
}