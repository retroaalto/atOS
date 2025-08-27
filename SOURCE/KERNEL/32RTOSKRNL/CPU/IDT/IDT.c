#include "IDT.h"
#include "../ISR/ISR.h"
#include "../../../../STD/ASM.h"
#include "../INTERRUPTS.h"
#include "../../../../STD/BINARY.h"

#define IDT_MEM_BASE MEM_IDT_BASE
#define IDT_MEM_END MEM_IDT_END
#define IDT_MEM_SIZE (IDT_MEM_END - IDT_MEM_BASE)
#if IDT_MEM_SIZE % 8 != 0
#error "IDT memory region size must be a multiple of 8 bytes"
#endif
#define IDT_COUNT 256

typedef struct __attribute__((packed)) {
    U16 base0;
    U16 selector;
    U8  reserved;
    U8  type_attr;
    U16 base1;
} IDTENTRY;

typedef struct __attribute__((packed)) {
    U16 size;
    IDTENTRY *base;
} IDTDESCRIPTOR;

static IDTENTRY idt[IDT_COUNT];
static IDTDESCRIPTOR idt_desc;


void idt_set_gate(U32 index, U0* handler, U16 sel, U8 flags) {
    idt[index].base0   = (U32)handler & 0xFFFF;
    idt[index].selector  = sel;
    idt[index].reserved  = 0;
    idt[index].type_attr = flags;
    idt[index].base1   = ((U32)handler >> 16) & 0xFFFF;
}

U0 IDT_INIT(U0) {
    SETUP_ISRS();
    idt_desc.size = sizeof(IDTENTRY) * IDT_COUNT - 1;
    idt_desc.base = idt;
    
    __asm__ volatile("lidt %0" : : "m"(idt_desc));
    return;
}
