#include "./IRQ.h"
#include "../IDT/IDT.h"
#include "../ISR/ISR.h"
#include "../GDT/GDT.h"
#include "../../../../STD/ASM.h"
#include <PIC.h>

// Initialize IRQs
void IRQ_INIT(void) {
    pic_remap(0x20, 0x28);                // Remap PICs
    return;
}

