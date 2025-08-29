#ifndef IDT_H
#define IDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"
#include "../ISR/ISR.h"
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

#define IDT_FLAG_PRESENT 0x80
#define INT_GATE_32 0x8E
void idt_set_gate(U32 index, U0* handler, U16 sel, U8 flags);
U0 IDT_INIT(U0);
IDTDESCRIPTOR *IDT_GET_PTR(void);

#endif
