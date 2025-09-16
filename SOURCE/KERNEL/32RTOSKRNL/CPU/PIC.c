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
    if (irq >= 8) _outb(0xA0, 0x20);
    _outb(0x20, 0x20);
}


// Remap PIC to avoid conflicts with CPU exceptions
void pic_remap(U8 offset1, U8 offset2) {
    
    // Mask all IRQs
    _outb(0x21, 0xFF);
    _outb(0xA1, 0xFF);
    
    // Start init
    _outb(0x20, 0x11); _io_wait();
    _outb(0xA0, 0x11); _io_wait();

    // Set new vector offsets
    _outb(0x21, offset1); _io_wait();
    _outb(0xA1, offset2); _io_wait();

    // Wiring
    _outb(0x21, 0x04); _io_wait();  // master has slave on IRQ2
    _outb(0xA1, 0x02); _io_wait();  // slave ID

    // 8086/88 mode
    _outb(0x21, 0x01); _io_wait();
    _outb(0xA1, 0x01); _io_wait();

    // Keep everything masked for now; caller will unmask specific IRQs later
    _outb(0x21, 0xFF);
    _outb(0xA1, 0xFF);
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