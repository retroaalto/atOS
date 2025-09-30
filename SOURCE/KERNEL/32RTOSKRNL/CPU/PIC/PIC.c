#include "./INTERRUPTS/INTERRUPTS.h"
#include "./PIC/PIC.h"
#include "../../../STD/ASM.h"




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
    if (irq >= 8) _outb(0xA0, 0x20);
    _outb(0x20, 0x20);
}


void pic_remap(U8 offset1, U8 offset2) {
    // Expect IF=0 on entry. Do not STI here.
    // Mask all IRQs immediately
    _outb(PIC1_DATA_PORT, 0xFF);
    _outb(PIC2_DATA_PORT, 0xFF);

    // ICW1: start initialization (edge-triggered, expect ICW4)
    _outb(PIC1_COMMAND_PORT, 0x11); _io_wait();
    _outb(PIC2_COMMAND_PORT, 0x11); _io_wait();
    
    // ICW2: vector offsets
    _io_wait();
    _outb(PIC1_DATA_PORT, offset1); _io_wait();    // e.g., 0x20
    _outb(PIC2_DATA_PORT, offset2); _io_wait();    // e.g., 0x28

    // ICW3: wiring
    _outb(PIC1_DATA_PORT, 0x04); _io_wait();       // master has slave on IRQ2
    _outb(PIC2_DATA_PORT, 0x02); _io_wait();       // slave identity

    // ICW4: 8086/88 mode
    _outb(PIC1_DATA_PORT, 0x01); _io_wait();
    _outb(PIC2_DATA_PORT, 0x01); _io_wait();

    // Leave masked; caller decides what to unmask later
    _outb(PIC1_DATA_PORT, 0xFF);
    _outb(PIC2_DATA_PORT, 0xFF);
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