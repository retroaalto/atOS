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

ISRHandler g_Handlers[IDT_COUNT];


#include "../../DRIVERS/VIDEO/VBE.h"
// This is the c-level exception handler
void isr_common_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    // for (;;) 
        // __asm__ volatile(
            // "cli\n\t"
            // "hlt\n\t"
        // );
}

void double_fault_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    for (;;) 
        __asm__ volatile(
            "cli\n\t"
            "hlt\n\t"
        );
}

void irq_common_handler(I32 num, U32 errcode) {
    (void)errcode; // Not used
    if(num >= 32 && num < 48) {
        pic_send_eoi(num - 32);
    }
}


// This function is called from assembly
void isr_dispatch_c(int vector, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    // For CPU exceptions (0-31) call isr_common_handler
    switch (vector)
    {
    case 8:
        double_fault_handler(vector, errcode);
        break;
    }
    if (vector < 32) {
        isr_common_handler(vector, errcode);
    } else if (vector >= 32 && vector < 48) {
        irq_common_handler(vector - 32, errcode);
    } else {
        isr_common_handler(vector, errcode);
    }
}

void irq_dispatch_c(int irq, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    // call irq_common_handler(irq)
    irq_common_handler(irq, errcode);
}


void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT) {
        g_Handlers[int_no] = handler;
    } else {
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
        irq32, irq33, irq34, irq35, irq36, irq37, irq38, 
        irq39, irq40, irq41, irq42, irq43, irq44, irq45, irq46, irq47,
        isr48
    };
    for(U32 i = 0; i < IDT_COUNT; i++) {
        if(i < 48)
            idt_set_gate(i, (U0*)isr[i], cs, flags);
        else
            idt_set_gate(i, (U0*)isr[49], cs, flags); // point to isr48 for now
    }
}

VOID SETUP_ISR_HANDLERS(VOID) {    
    for(int i = 0; i < IDT_COUNT; i++) {
        if(i < 32) {
            ISR_REGISTER_HANDLER(i, isr_common_handler); // CPU exceptions
        } else if(i >= 32 && i < 48) {
            ISR_REGISTER_HANDLER(i, irq_common_handler);
        } else {
            ISR_REGISTER_HANDLER(i, isr_common_handler); // Reserved / unused
        }
    }
}
