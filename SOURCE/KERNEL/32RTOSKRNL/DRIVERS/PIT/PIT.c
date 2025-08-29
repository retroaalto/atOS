#include "./PIT.h"
#include <PIC.h>
#include "../../../STD/ASM.h"
#include "../../CPU/ISR/ISR.h"
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

void pit_set_frequency(U32 freq) {
    U32 divisor = PIT_FREQUENCY / freq;

    // Send command byte: channel 0, access low+high, mode 3 (square wave), binary
    _outb(PIT_COMMAND, 0x36);

    // Send divisor low byte, then high byte
    _outb(PIT_CHANNEL0, (U8)(divisor & 0xFF));
    _outb(PIT_CHANNEL0, (U8)((divisor >> 8) & 0xFF));
}

static U32 ticks = 0;
static U32 hz = 100;
void pit_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    ticks++;
    return;
}

U0 PIT_INIT(U0) {
    hz = 100;
    pit_set_frequency(hz);
    ISR_REGISTER_HANDLER(PIC_REMAP_OFFSET + 0, pit_handler);
    PIC_Unmask(0);
    ticks = 0;
}

U32 *PIT_GET_TICKS_PTR() {
    return &ticks;
}

U32 *PIT_GET_HZ_PTR() {
    return &hz;
}