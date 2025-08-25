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
#include "../../../../STD/ASM.h"

ISRHandler g_IRQHandlers[16];

void IRQ_RegisterHandler(int irq, ISRHandler handler) {
    g_IRQHandlers[irq] = handler;
}

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT    0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT    0xA1

#define PIC_ICW1_ICW4   0x01
#define PIC_ICW1_INITIALIZE 0x10
#define PIC_ICW4_8086  0x01
#define PIC_REMAP_OFFSET 0x20


// Send End-of-Interrupt (EOI) to PICs
void pic_send_eoi(U8 irq) {
    if(irq >= 8) 
        outb(PIC2_COMMAND_PORT, 0x20); // Slave PIC
    outb(PIC1_COMMAND_PORT, 0x20); // Master PIC
}


void irq_default_handler(struct regs* r) {
    I32 irq = r->int_no - PIC_REMAP_OFFSET;
    if(irq >= 0 && irq < 16) {
        pic_send_eoi(irq);
    }
}




// Remap the PIC to avoid conflicts with CPU exceptions
void pic_remap() {
    U8 a1, a2;
    a1 = inb(PIC1_DATA_PORT);
    a2 = inb(PIC2_DATA_PORT);

    outb(PIC1_COMMAND_PORT, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND_PORT, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA_PORT, 0x20); // Offset for PIC1
    io_wait();
    outb(PIC2_DATA_PORT, 0x28); // Offset for PIC2
    io_wait();

    outb(PIC1_DATA_PORT, 4);
    io_wait();
    outb(PIC2_DATA_PORT, 2);
    io_wait();

    outb(PIC1_DATA_PORT, PIC_ICW4_8086);
    io_wait();
    outb(PIC2_DATA_PORT, PIC_ICW4_8086);
    io_wait();

    outb(PIC1_DATA_PORT, a1);
    outb(PIC2_DATA_PORT, a2);
}

U0 IRQ_INIT(U0) {
    for(U32 i = 0; i < 16; i++) {
        ISR_REGISTER_HANDLER(PIC_REMAP_OFFSET + i, irq_default_handler);
    }
    SETUP_ISR_HANDLERS();
}
