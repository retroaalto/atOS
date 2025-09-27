#include "./IRQ.h"
#include "../IDT/IDT.h"
#include "../ISR/ISR.h"
#include "../GDT/GDT.h"
#include "../../../../STD/ASM.h"
#include "./PIC.h"


// Initialize IRQs
void IRQ_INIT(void) {
    pic_remap(0x20, 0x20);                // Remap PICs
    for(U8 i = 0; i < 16; i++) {
        PIC_Mask(i);
    }
    return;
}

