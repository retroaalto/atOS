#include "./IRQ.h"
#include "../IDT/IDT.h"
#include "../ISR/ISR.h"
#include "../GDT/GDT.h"
#include "../../../../STD/ASM.h"
#include "./PIC/PIC.h"


// Initialize IRQs
void IRQ_INIT(void) {
    pic_remap(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET2); // Remap PICs
    return;
}

