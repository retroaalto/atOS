#include <DRIVERS/BEEPER/BEEPER.h>
#include <STD/ASM.h>

#define PIT_BASE_FREQUENCY 1193182
#define PIT_CMD_PORT       0x43
#define PIT_CHL2_DATA_PORT 0x42
#define GP_PORT            0x61
#define PIT_MODE3_CH2      0xB6


VOID SET_BEEPER_FREQUENCY(U32 f) {
    if (f == 0) {
        STOP_BEEPER();
        return;
    }
    if (f < 18) f = 18;
    if (f > 1193182) f = 1193182;


    U16 divisor = (U16)(PIT_BASE_FREQUENCY / f);
    if (divisor == 0) divisor = 1;
    _outb(PIT_CMD_PORT, PIT_MODE3_CH2);
    _outb(PIT_CHL2_DATA_PORT, (U8)(divisor & 0xFF));
    _outb(PIT_CHL2_DATA_PORT, (U8)((divisor >> 8) & 0xFF));
}


VOID START_BEEPER() {
    U8 port_data = _inb(GP_PORT);
    port_data |= 0x03; // set bits 0 and 1
    _outb(GP_PORT, port_data);
}

VOID STOP_BEEPER() {
    U8 port_data = _inb(GP_PORT);
    port_data &= ~0x02; // disconnect speaker output
    port_data &= ~0x01; // disable PIT channel 2 gate
    _outb(GP_PORT, port_data);
}