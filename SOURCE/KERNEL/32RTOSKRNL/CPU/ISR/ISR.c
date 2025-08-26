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

#warning REMOVE ME
#include "../../DRIVERS/VIDEO/VBE.h"

// This is the c-level exception handler
__attribute__((noreturn))
void exception_handler(struct regs* r){
    (void)r;
    // ASM_VOLATILE("cli");
    // for(;;) { ASM_VOLATILE("hlt"); }  // infinite halt
}

// This function is called from assembly
__attribute__((noreturn))
void isr_default_handler(struct regs* r) {
    if(g_Handlers[r->int_no] != 0) {
        g_Handlers[r->int_no](r); // Calls c-level handler if one is registered
    } else {
        ASM_VOLATILE("cli");
        for(;;) { ASM_VOLATILE("hlt"); }  // infinite halt
    }
}

__attribute__((noreturn))
void irq_default_handler(struct regs* r) {
    U8 irq = r->int_no - 32; // IRQs mapped to 32–47
    if (irq < 16 && g_IRQHandlers[irq]) {
        g_IRQHandlers[irq](r);  // Call custom handler
    }

    pic_send_eoi(irq);          // Always notify PIC

    for (;;) { asm volatile("hlt"); } // Halt CPU if no return
}


void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT) {
        g_Handlers[int_no] = handler;
        IDT_ENABLEGATE(int_no);
    } else {
        IDT_DISABLEGATE(int_no);
    }
}
#define fdf(x) x

U0 SETUP_ISRS(U0) {
    idt_set_gate(0, fdf(isr0), KCODE_SEL, INT_GATE_32);
    idt_set_gate(1, fdf(isr1), KCODE_SEL, INT_GATE_32);
    idt_set_gate(2, fdf(isr2), KCODE_SEL, INT_GATE_32);
    idt_set_gate(3, fdf(isr3), KCODE_SEL, INT_GATE_32);
    idt_set_gate(4, fdf(isr4), KCODE_SEL, INT_GATE_32);
    idt_set_gate(5, fdf(isr5), KCODE_SEL, INT_GATE_32);
    idt_set_gate(6, fdf(isr6), KCODE_SEL, INT_GATE_32);
    idt_set_gate(7, fdf(isr7), KCODE_SEL, INT_GATE_32);
    idt_set_gate(8, fdf(isr8), KCODE_SEL, INT_GATE_32);
    idt_set_gate(9, fdf(isr9), KCODE_SEL, INT_GATE_32);
    idt_set_gate(10, fdf(isr10), KCODE_SEL, INT_GATE_32);
    idt_set_gate(11, fdf(isr11), KCODE_SEL, INT_GATE_32);
    idt_set_gate(12, fdf(isr12), KCODE_SEL, INT_GATE_32);
    idt_set_gate(13, fdf(isr13), KCODE_SEL, INT_GATE_32);
    idt_set_gate(14, fdf(isr14), KCODE_SEL, INT_GATE_32);
    idt_set_gate(15, fdf(isr15), KCODE_SEL, INT_GATE_32);
    idt_set_gate(16, fdf(isr16), KCODE_SEL, INT_GATE_32);
    idt_set_gate(17, fdf(isr17), KCODE_SEL, INT_GATE_32);
    idt_set_gate(18, fdf(isr18), KCODE_SEL, INT_GATE_32);
    idt_set_gate(19, fdf(isr19), KCODE_SEL, INT_GATE_32);
    idt_set_gate(20, fdf(isr20), KCODE_SEL, INT_GATE_32);
    idt_set_gate(21, fdf(isr21), KCODE_SEL, INT_GATE_32);
    idt_set_gate(22, fdf(isr22), KCODE_SEL, INT_GATE_32);
    idt_set_gate(23, fdf(isr23), KCODE_SEL, INT_GATE_32);
    idt_set_gate(24, fdf(isr24), KCODE_SEL, INT_GATE_32);
    idt_set_gate(25, fdf(isr25), KCODE_SEL, INT_GATE_32);
    idt_set_gate(26, fdf(isr26), KCODE_SEL, INT_GATE_32);
    idt_set_gate(27, fdf(isr27), KCODE_SEL, INT_GATE_32);
    idt_set_gate(28, fdf(isr28), KCODE_SEL, INT_GATE_32);
    idt_set_gate(29, fdf(isr29), KCODE_SEL, INT_GATE_32);
    idt_set_gate(30, fdf(isr30), KCODE_SEL, INT_GATE_32);
    idt_set_gate(31, fdf(isr31), KCODE_SEL, INT_GATE_32);
    idt_set_gate(32, fdf(isr32), KCODE_SEL, INT_GATE_32);
    idt_set_gate(33, fdf(isr33), KCODE_SEL, INT_GATE_32);
    idt_set_gate(34, fdf(isr34), KCODE_SEL, INT_GATE_32);
    idt_set_gate(35, fdf(isr35), KCODE_SEL, INT_GATE_32);
    idt_set_gate(36, fdf(isr36), KCODE_SEL, INT_GATE_32);
    idt_set_gate(37, fdf(isr37), KCODE_SEL, INT_GATE_32);
    idt_set_gate(38, fdf(isr38), KCODE_SEL, INT_GATE_32);
    idt_set_gate(39, fdf(isr39), KCODE_SEL, INT_GATE_32);
    idt_set_gate(40, fdf(isr40), KCODE_SEL, INT_GATE_32);
    idt_set_gate(41, fdf(isr41), KCODE_SEL, INT_GATE_32);
    idt_set_gate(42, fdf(isr42), KCODE_SEL, INT_GATE_32);
    idt_set_gate(43, fdf(isr43), KCODE_SEL, INT_GATE_32);
    idt_set_gate(44, fdf(isr44), KCODE_SEL, INT_GATE_32);
    idt_set_gate(45, fdf(isr45), KCODE_SEL, INT_GATE_32);
    idt_set_gate(46, fdf(isr46), KCODE_SEL, INT_GATE_32);
    idt_set_gate(47, fdf(isr47), KCODE_SEL, INT_GATE_32);
    idt_set_gate(48, fdf(isr48), KCODE_SEL, INT_GATE_32);
    idt_set_gate(49, fdf(isr49), KCODE_SEL, INT_GATE_32);
    idt_set_gate(50, fdf(isr50), KCODE_SEL, INT_GATE_32);
    for(U32 i = 51; i < IDT_COUNT; i++) {
        idt_set_gate(i, fdf(isr51), KCODE_SEL, INT_GATE_32);
    }   

}

VOID SETUP_ISR_HANDLERS(VOID) {    
    for(int i = 0; i < IDT_COUNT; i++) {
        if(i < 32) {
            ISR_REGISTER_HANDLER(i, exception_handler); // CPU exceptions
        } else if(i >= 32 && i < 48) {
            ISR_REGISTER_HANDLER(i, irq_default_handler);
        } else {
            ISR_REGISTER_HANDLER(i, exception_handler); // Reserved / unused
        }
    }
}


