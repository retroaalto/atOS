#include "./INTERRUPTS.h"
#include "./PIC.h"
#include "../../../STD/ASM.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1


enum {
    PIC_ICW1_ICW4           = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_INTERVAL4      = 0x04,
    PIC_ICW1_LEVEL          = 0x08,
    PIC_ICW1_INITIALIZE     = 0x10
} PIC_ICW1;

enum {
    PIC_ICW4_8086           = 0x1,
    PIC_ICW4_AUTO_EOI       = 0x2,
    PIC_ICW4_BUFFER_MASTER  = 0x4,
    PIC_ICW4_BUFFER_SLAVE   = 0x0,
    PIC_ICW4_BUFFERRED      = 0x8,
    PIC_ICW4_SFNM           = 0x10,
} PIC_ICW4;

enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;


// void PIC_Disable() {
//     _outb(PIC1_DATA_PORT, 0xFF);
//     _outb(PIC2_DATA_PORT, 0xFF);
// }

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
    if(irq < 8) {
        U8 mask = _inb(PIC1_DATA_PORT);
        _outb(PIC1_DATA_PORT, mask & ~(1 << irq));
    } else {
        U8 mask = _inb(PIC2_DATA_PORT);
        _outb(PIC2_DATA_PORT, mask & ~(1 << (irq - 8)));
    }
}


void pic_send_eoi(U8 irq) {
    if (irq >= 8) {
        _outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT); // Slave PIC
    }
    _outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);     // Master PIC
}


// Remap PIC to avoid conflicts with CPU exceptions
void pic_remap(U8 offset1, U8 offset2) {
    U8 a1 = _inb(PIC1_DATA_PORT);
    U8 a2 = _inb(PIC2_DATA_PORT);

    _outb(PIC1_COMMAND_PORT, 0x11);  // ICW1: start initialization
    _io_wait();
    _outb(PIC2_COMMAND_PORT, 0x11);
    _io_wait();

    _outb(PIC1_DATA_PORT, offset1);     // ICW2: offset 0x20 for master
    _io_wait();
    _outb(PIC2_DATA_PORT, offset2);     // ICW2: offset 0x28 for slave
    _io_wait();

    _outb(PIC1_DATA_PORT, 4);         // ICW3: master has slave at IRQ2
    _io_wait();
    _outb(PIC2_DATA_PORT, 2);         // ICW3: slave identity
    _io_wait();

    _outb(PIC1_DATA_PORT, 1);         // ICW4: 8086 mode
    _io_wait();
    _outb(PIC2_DATA_PORT, 1);
    _io_wait();

    _outb(PIC1_DATA_PORT, a1);        // restore saved masks
    _outb(PIC2_DATA_PORT, a2);
}

// uint16_t PIC_ReadIRQRequestRegister() {
//     _outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
//     _outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);

//     return ((uint16_t)_inb(PIC1_COMMAND_PORT)) | (((uint16_t)_inb(PIC2_COMMAND_PORT)) << 8); 
// }

// uint16_t PIC_ReadInServiceRegister() {
//     _outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
//     _outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);

//     return ((uint16_t)_inb(PIC1_COMMAND_PORT)) | (((uint16_t)_inb(PIC2_COMMAND_PORT)) << 8); 
// }