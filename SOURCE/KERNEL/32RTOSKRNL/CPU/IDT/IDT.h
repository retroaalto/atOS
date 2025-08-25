#ifndef IDT_H
#define IDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"
#include "../ISR/ISR.h"

#define IDT_MEM_BASE MEM_IDT_BASE
#define IDT_MEM_END MEM_IDT_END
#define IDT_MEM_SIZE (IDT_MEM_END - IDT_MEM_BASE)
#if IDT_MEM_SIZE % 8 != 0
#error "IDT memory region size must be a multiple of 8 bytes"
#endif
#define IDT_COUNT 256
#define INT_GATE_32 0x8E

typedef struct __attribute__((packed)) {
    U16 offset0;
    U16 selector;
    U8  zero;
    U8  type_attr;
    U16 offset1;
} IDTENTRY;

typedef struct __attribute__((packed)) {
    U16 size;
    U32* offset;
} IDTDESCRIPTOR;

void idt_set_gate(U32 index, U32 handler, U16 sel, U8 flags);
void IDT_INIT(void);

#endif
