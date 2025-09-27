/*+++
    Source/KERNEL/32RTOSKRNL/CPU/ISR/ISR.c - Interrupt Service Routine (ISR) Implementation

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt service routine (ISR) implementation for atOS.

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
#include "../../../../STD/ASM.h"
#include "../INTERRUPTS.h"
#include "../PIC.h"
#include "../../DRIVERS/PIT/PIT.h"


static ISRHandler g_Handlers[IDT_COUNT] = { 0 };

ISRHandler *ISR_GET_PTR(void) {
    return g_Handlers;
}

// This is the c-level exception handler
void isr_common_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    HLT;
}

void double_fault_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    HLT;
}


void irq_common_handler(I32 vector, U32 errcode) {
    (void)errcode;
    if (g_Handlers[vector]) g_Handlers[vector](vector, 0);
    int irq = vector - PIC_REMAP_OFFSET; // 0x20
    if ((unsigned)irq < 16) pic_send_eoi(irq);

}



// This function is called from assembly
void isr_dispatch_c(int vector, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    if(g_Handlers[vector]) {
        g_Handlers[vector](vector, errcode);
        return;
    }
}

void irq_dispatch_c(int vector, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    irq_common_handler(vector, errcode);

}


void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT) {
        g_Handlers[int_no] = handler;
    }
}

U0 SETUP_ISRS(U0) {
    U16 cs = 0x08; // code selector
    U8 flags = 0x8E; // present, ring0, 32-bit interrupt gate
    void *isr[49] = {
        isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,
        isr8,isr9,isr10,isr11,isr12,isr13,isr14,isr15,
        isr16,isr17,isr18,isr19,isr20,isr21,isr22,isr23,
        isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31,
        irq32, irq33, irq34, irq35, irq36, irq37, irq38, irq39,
        irq40, irq41, irq42, irq43, irq44, irq45, irq46, irq47,
        isr48
    };
    for(U32 i = 0; i < IDT_COUNT; i++) {
        if (i < 49) idt_set_gate(i, isr[i], cs, flags);
        else        idt_set_gate(i, isr[48], cs, flags);
    }
}
VOID SETUP_ISR_HANDLERS(VOID) {
    for(int i = 0; i < IDT_COUNT; i++) {
        if(i < 32) {
            switch(i) {
                case 8:
                    ISR_REGISTER_HANDLER(i, double_fault_handler);
                    break;
                default:
                    ISR_REGISTER_HANDLER(i, isr_common_handler); // CPU exceptions
            }
        } else if(i >= 32 && i < 48) {
            ISR_REGISTER_HANDLER(i, irq_common_handler); // No default handler
        } else {
            ISR_REGISTER_HANDLER(i, handlers[1]); // Reserved / unused
        }
    }
}