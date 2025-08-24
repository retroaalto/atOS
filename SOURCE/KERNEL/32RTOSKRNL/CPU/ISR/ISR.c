/*+++
    Source/KERNEL/32RTOSKRNL/CPU/ISR/ISR.c - Interrupt Service Routine (ISR) Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt service routine (ISR) implementation for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. ISR handling functions added.
REMARKS
---*/
#include "./ISR.h"
#include "../IRQ/IRQ.h"
#include "../IDT/IDT.h"

ISRHandler g_Handlers[IDT_COUNT];

void isr_default_handler(struct regs* r) {
}
void isr0_handler(struct regs* r) {
    isr_default_handler(r);
}

// Generic C-level ISR handler
void isr_handler(struct regs* r) {
    if(r->int_no < IDT_COUNT && g_Handlers[r->int_no])
        g_Handlers[r->int_no](r);
    // Do not hlt() here if you want drawing to work
}

VOID SETUP_ISR_HANDLERS(VOID) {
    for(int i = 0; i < IDT_COUNT; i++) {
        if(i < 32) {
            ISR_REGISTER_HANDLER(i, isr_default_handler); // CPU exceptions
        } else if(i >= 32 && i < 48) {
            ISR_REGISTER_HANDLER(i, irq_default_handler); // Hardware IRQs
        } else {
            ISR_REGISTER_HANDLER(i, isr_default_handler); // Reserved / unused
        }
    }
}
ISR_STUB(0);
ISR_STUB(1);
ISR_STUB(2);
ISR_STUB(3);