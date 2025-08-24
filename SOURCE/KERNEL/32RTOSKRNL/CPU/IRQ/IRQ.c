/*+++
    Source/KERNEL/32RTOSKRNL/CPU/IRQ/IRQ.c - Interrupt Request (IRQ) Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt request (IRQ) implementation for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. IRQ remapping functions added.
REMARKS
---*/

#include "./IRQ.h"
#include "../IDT/IDT.h"
#include "../ISR/ISR.h"
#include "../GDT/GDT.h"

// Generate naked ISR stubs for IRQs 0-15
ISR_STUB(32) // IRQ0 - System Timer
ISR_STUB(33) // IRQ1 - Keyboard
ISR_STUB(34) // IRQ2 - Cascade / Reserved
ISR_STUB(35) // IRQ3 - COM2
ISR_STUB(36) // IRQ4 - COM1
ISR_STUB(37) // IRQ5 - LPT2 / Sound Card
ISR_STUB(38) // IRQ6 - Floppy Disk
ISR_STUB(39) // IRQ7 - LPT1 / Parallel Port
ISR_STUB(40) // IRQ8 - RTC / CMOS
ISR_STUB(41) // IRQ9 - ACPI / Redirected IRQ2
ISR_STUB(42) // IRQ10 - General Purpose / Network
ISR_STUB(43) // IRQ11 - General Purpose / Network
ISR_STUB(44) // IRQ12 - Mouse
ISR_STUB(45) // IRQ13 - FPU / Coprocessor
ISR_STUB(46) // IRQ14 - Primary ATA
ISR_STUB(47) // IRQ15 - Secondary ATA

// Send End-of-Interrupt (EOI) to PICs
void pic_send_eoi(U8 irq) {
    if(irq >= 8) 
        outb(PIC2_CMD, 0x20); // Slave PIC
    outb(PIC1_CMD, 0x20);     // Master PIC
}

// Default IRQ handler called from C
void irq_default_handler(struct regs* r) {
    if(r->int_no >= 32 && r->int_no < 48) {
        pic_send_eoi(r->int_no - 32);
    }
}

// Remap the PIC to avoid conflicts with CPU exceptions
void pic_remap() {
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

    outb(PIC1_DATA, 0x20); // Master PIC offset (IRQ0-7 -> INT 32-39)
    outb(PIC2_DATA, 0x28); // Slave PIC offset  (IRQ8-15 -> INT 40-47)

    outb(PIC1_DATA, 4);    // Tell Master about Slave at IRQ2
    outb(PIC2_DATA, 2);    // Tell Slave its cascade identity

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, 0); // Unmask all IRQs
    outb(PIC2_DATA, 0);
}

// Initialize IRQs and set IDT gates
VOID IRQ_INIT(void) {
    pic_remap();

    // Map IRQ stubs into IDT entries
    for(U8 i = 0; i < 16; i++) {
        U32 isr_addr = 0;
        switch(i) {
            case 0:  isr_addr = (U32)&isr_32; break;
            case 1:  isr_addr = (U32)&isr_33; break;
            case 2:  isr_addr = (U32)&isr_34; break;
            case 3:  isr_addr = (U32)&isr_35; break;
            case 4:  isr_addr = (U32)&isr_36; break;
            case 5:  isr_addr = (U32)&isr_37; break;
            case 6:  isr_addr = (U32)&isr_38; break;
            case 7:  isr_addr = (U32)&isr_39; break;
            case 8:  isr_addr = (U32)&isr_40; break;
            case 9:  isr_addr = (U32)&isr_41; break;
            case 10: isr_addr = (U32)&isr_42; break;
            case 11: isr_addr = (U32)&isr_43; break;
            case 12: isr_addr = (U32)&isr_44; break;
            case 13: isr_addr = (U32)&isr_45; break;
            case 14: isr_addr = (U32)&isr_46; break;
            case 15: isr_addr = (U32)&isr_47; break;
        }
        idt_set_gate(32 + i, isr_addr, KCODE_SEL, INT_GATE_32);
    }
}
