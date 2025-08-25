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
#include "../../../../STD/ASM.h"
#include "../INTERRUPTS.h"

ISRHandler g_Handlers[IDT_COUNT];

void isr_default_handler(struct regs* r) {
     if(g_Handlers[r->int_no] != 0) {
         g_Handlers[r->int_no](r);
    }else {
        ASM_VOLATILE("hlt");
    }
}

void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT)
        g_Handlers[int_no] = handler;
}


U0 SETUP_ISRS() {
    idt_set_gate(0, (U32)&isr0, 0x08, 0x8E);
    idt_set_gate(1, (U32)&isr1, 0x08, 0x8E);
    idt_set_gate(2, (U32)&isr2, 0x08, 0x8E);
    idt_set_gate(3, (U32)&isr3, 0x08, 0x8E);
    idt_set_gate(4, (U32)&isr4, 0x08, 0x8E);
    idt_set_gate(5, (U32)&isr5, 0x08, 0x8E);
    idt_set_gate(6, (U32)&isr6, 0x08, 0x8E);
    idt_set_gate(7, (U32)&isr7, 0x08, 0x8E);
    idt_set_gate(8, (U32)&isr8, 0x08, 0x8E);
    idt_set_gate(9, (U32)&isr9, 0x08, 0x8E);
    idt_set_gate(10, (U32)isr10, 0x08, 0x8E);
    idt_set_gate(11, (U32)isr11, 0x08, 0x8E);
    idt_set_gate(12, (U32)isr12, 0x08, 0x8E);
    idt_set_gate(13, (U32)isr13, 0x08, 0x8E);
    idt_set_gate(14, (U32)isr14, 0x08, 0x8E);
    idt_set_gate(15, (U32)isr15, 0x08, 0x8E);
    idt_set_gate(16, (U32)isr16, 0x08, 0x8E);
    idt_set_gate(17, (U32)isr17, 0x08, 0x8E);
    idt_set_gate(18, (U32)isr18, 0x08, 0x8E);
    idt_set_gate(19, (U32)isr19, 0x08, 0x8E);
    idt_set_gate(20, (U32)isr20, 0x08, 0x8E);
    idt_set_gate(21, (U32)isr21, 0x08, 0x8E);
    idt_set_gate(22, (U32)isr22, 0x08, 0x8E);
    idt_set_gate(23, (U32)isr23, 0x08, 0x8E);
    idt_set_gate(24, (U32)isr24, 0x08, 0x8E);
    idt_set_gate(25, (U32)isr25, 0x08, 0x8E);
    idt_set_gate(26, (U32)isr26, 0x08, 0x8E);
    idt_set_gate(27, (U32)isr27, 0x08, 0x8E);
    idt_set_gate(28, (U32)isr28, 0x08, 0x8E);
    idt_set_gate(29, (U32)isr29, 0x08, 0x8E);
    idt_set_gate(30, (U32)isr30, 0x08, 0x8E);
    idt_set_gate(31, (U32)isr31, 0x08, 0x8E);
    idt_set_gate(32, (U32)isr32, 0x08, 0x8E);
    idt_set_gate(33, (U32)isr33, 0x08, 0x8E);
    idt_set_gate(34, (U32)isr34, 0x08, 0x8E);
    idt_set_gate(35, (U32)isr35, 0x08, 0x8E);
    idt_set_gate(36, (U32)isr36, 0x08, 0x8E);
    idt_set_gate(37, (U32)isr37, 0x08, 0x8E);
    idt_set_gate(38, (U32)isr38, 0x08, 0x8E);
    idt_set_gate(39, (U32)isr39, 0x08, 0x8E);
    idt_set_gate(40, (U32)isr40, 0x08, 0x8E);
    idt_set_gate(41, (U32)isr41, 0x08, 0x8E);
    idt_set_gate(42, (U32)isr42, 0x08, 0x8E);
    idt_set_gate(43, (U32)isr43, 0x08, 0x8E);
    idt_set_gate(44, (U32)isr44, 0x08, 0x8E);
    idt_set_gate(45, (U32)isr45, 0x08, 0x8E);
    idt_set_gate(46, (U32)isr46, 0x08, 0x8E);
    idt_set_gate(47, (U32)isr47, 0x08, 0x8E);
    idt_set_gate(48, (U32)isr48, 0x08, 0x8E);
    idt_set_gate(49, (U32)isr49, 0x08, 0x8E);
    idt_set_gate(50, (U32)isr50, 0x08, 0x8E);
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


