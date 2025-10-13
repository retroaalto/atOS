#include "./INTERRUPTS/INTERRUPTS.h"
#include "./PIC/PIC.h"
#include "../../../STD/ASM.h"

enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
} PIC_CMD;

void PIC_Mask(int irq) {
    if(irq < 8) {
        U8 mask = _inb(PIC1_DATA_PORT);
        _outb(PIC1_DATA_PORT, mask | (1 << irq));
    } else {
        U8 mask = _inb(PIC2_DATA_PORT);
        _outb(PIC2_DATA_PORT, mask | (1 << (irq - 8)));
    }
}

void PIC_Unmask(int irq) {
    if (irq < 8) {
        // Master PIC
        U8 mask = _inb(PIC1_DATA_PORT);
        mask &= ~(1 << irq);       // Clear the bit to unmask
        _outb(PIC1_DATA_PORT, mask);
    } else {
        // Slave PIC
        int slave_irq = irq - 8;

        // Unmask the slave IRQ
        U8 slave_mask = _inb(PIC2_DATA_PORT);
        slave_mask &= ~(1 << slave_irq); 
        _outb(PIC2_DATA_PORT, slave_mask);

        // Ensure master cascade line (IRQ2) is always unmasked
        U8 master_mask = _inb(PIC1_DATA_PORT);
        master_mask &= ~(1 << 2);  // Clear IRQ2
        _outb(PIC1_DATA_PORT, master_mask);
    }
}


void pic_send_eoi(U8 irq) {
    if (irq >= 8) {
        _outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT); _io_wait();
    }
    _outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT); _io_wait();
}


void pic_remap(U8 offset1, U8 offset2) {
    // Mask all IRQs immediately
    _outb(PIC1_DATA_PORT, 0xFF);
    _outb(PIC2_DATA_PORT, 0xFF);

    // ICW1: start initialization (edge-triggered, expect ICW4)
    _outb(PIC1_COMMAND_PORT, 0x11); _io_wait();
    _outb(PIC2_COMMAND_PORT, 0x11); _io_wait();
    
    // ICW2: vector offsets
    _outb(PIC1_DATA_PORT, offset1); _io_wait();    // e.g., 0x20
    _outb(PIC2_DATA_PORT, offset2); _io_wait();    // e.g., 0x28

    // ICW3: wiring
    _outb(PIC1_DATA_PORT, 0x04); _io_wait();       // master has slave on IRQ2
    _outb(PIC2_DATA_PORT, 0x02); _io_wait();       // slave identity

    // ICW4: 8086/88 mode
    _outb(PIC1_DATA_PORT, 0x01); _io_wait();
    _outb(PIC2_DATA_PORT, 0x01); _io_wait();

    // Unmask cascade
    _outb(PIC1_DATA_PORT, 0xFB);
    _outb(PIC2_DATA_PORT, 0xFF);       // All slave IRQs masked
}
