#include "IDT.h"
#include "../GDT/GDT.h"
#include "../ISR/ISR.h"

// Pointer to memory-mapped IDT
static IDTDESCRIPTOR idtr;

// Low-level function to set a gate in memory
void idt_set_gate(I32 idx, U32 handler_addr, U16 sel, U8 type_attr) {
    IDTENTRY* gate = &IDT_PTR[idx];
    gate->OffsetLow  = handler_addr & 0xFFFF;
    gate->Selector   = sel;
    gate->Zero       = 0;
    gate->TypeAttr   = type_attr;
    gate->OffsetHigh = (handler_addr >> 16) & 0xFFFF;
}

// Register a C-level ISR
void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT)
        g_Handlers[int_no] = handler;
}

// Initialize the IDT
void IDT_INIT(void) {
    for(int i = 0; i < IDT_COUNT; i++)
        idt_set_gate(i, isr_default_handler, KCODE_SEL, INT_GATE_32);

    idtr.size = IDT_COUNT * sizeof(IDTENTRY) - 1;
    idtr.offset = IDT_MEM_BASE;
    __asm__ volatile("lidt %0" : : "m"(idtr));
}
