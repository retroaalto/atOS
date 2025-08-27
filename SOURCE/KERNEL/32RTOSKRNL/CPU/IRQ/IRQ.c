#include "./IRQ.h"
#include "../IDT/IDT.h"
#include "../ISR/ISR.h"
#include "../GDT/GDT.h"
#include "../../../../STD/ASM.h"

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define PIC_ICW1_ICW4 0x01
#define PIC_ICW1_INIT 0x10
#define PIC_ICW4_8086 0x01

#define PIC_EOI     0x20

void pic_send_eoi(U8 irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI); // Slave PIC
    }
    outb(PIC1_CMD, PIC_EOI);     // Master PIC
}


// Remap PIC to avoid conflicts with CPU exceptions
void pic_remap(U8 offset1, U8 offset2) {
    U8 a1 = 0;
    inb(PIC1_DATA, a1);
    U8 a2 = 0;
    inb(PIC2_DATA, a2);

    outb(PIC1_CMD, 0x11);  // ICW1: start initialization
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();

    outb(PIC1_DATA, offset1);     // ICW2: offset 0x20 for master
    io_wait();
    outb(PIC2_DATA, offset2);     // ICW2: offset 0x28 for slave
    io_wait();

    outb(PIC1_DATA, 4);         // ICW3: master has slave at IRQ2
    io_wait();
    outb(PIC2_DATA, 2);         // ICW3: slave identity
    io_wait();

    outb(PIC1_DATA, 1);         // ICW4: 8086 mode
    io_wait();
    outb(PIC2_DATA, 1);
    io_wait();

    outb(PIC1_DATA, a1);        // restore saved masks
    outb(PIC2_DATA, a2);
}


// Initialize IRQs
void IRQ_INIT(void) {
    pic_remap(0x20, 0x28);                // Remap PICs
}

