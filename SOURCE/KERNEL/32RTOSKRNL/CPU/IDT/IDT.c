#include "IDT.h"
#include "../GDT/GDT.h"
#include "../ISR/ISR.h"

// Pointer to memory-mapped IDT
static IDTDESCRIPTOR idtr;

void idt_set_gate(U32 index, U32 handler, U16 sel, U8 flags) {
    IDTENTRY* entry = (IDTENTRY*)(IDT_MEM_BASE + index * sizeof(IDTENTRY));
    entry->offset0 = (U16)(handler & 0xFFFF);
    entry->selector = sel; // Kernel code segment selector
    entry->zero = 0;
    entry->type_attr = flags; // Present, kernel mode, interrupt gate
    entry->offset1 = (U16)((handler >> 16) & 0xFFFF);
}

// Initialize the IDT
void IDT_INIT(void) {
    idtr.size = IDT_MEM_SIZE;
    idtr.offset = IDT_MEM_BASE;

    __asm__ volatile("lidt %0" : : "m"(idtr));
}
