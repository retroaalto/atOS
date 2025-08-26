/*+++
    Source/KERNEL/32RTOSKRNL/CPU/GDT/GDT.c - GDT Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit GDT and IDT implementations for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains GDT structures and constants.

REMARKS

    Only for kernel usage.

---*/
#include "GDT.h"

typedef struct __attribute__((packed)) {
    U16 limit0;
    U16 base0;
    U8  base1;
    U8  access;
    U8  granularity;
    U8  base2;
} GDTENTRY;

typedef struct __attribute__((packed)) {
    U16 limit;
    GDTENTRY *base;
} GDTDESCRIPTOR;

#define GDT_ENTRY_COUNT 3

#define READABLE_RNG0_KRNL 0x9A
#define WRITABLE_RNG0_KRNL 0x92
#define GRANULARITY 0xCF

GDTENTRY *g_GDT = (GDTENTRY *)(MEM_GDT_BASE);
GDTDESCRIPTOR *g_GDTDescriptor = (GDTDESCRIPTOR *)(MEM_GDT_BASE + sizeof(GDTENTRY) * GDT_ENTRY_COUNT);

void gdt_set_gate(U32 num, U32 base, U32 limit, U8 access, U8 gran) {
    g_GDT[num].base0       = base & 0xFFFF;
    g_GDT[num].base1       = (base >> 16) & 0xFF;
    g_GDT[num].base2       = (base >> 24) & 0xFF;
    g_GDT[num].limit0      = limit & 0xFFFF;
    g_GDT[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    g_GDT[num].access      = access;
}

U0 GDT_INIT(U0) {
    // __asm__ volatile("hlt");
    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);                  
    // Kernel code segment
    gdt_set_gate(1, 0, 0xFFFFF, READABLE_RNG0_KRNL, GRANULARITY); 
    // Kernel data segment
    gdt_set_gate(2, 0, 0xFFFFF, WRITABLE_RNG0_KRNL, GRANULARITY); 

    g_GDTDescriptor->limit = GDT_ENTRY_COUNT * sizeof(GDTENTRY) - 1;
    g_GDTDescriptor->base  = g_GDT;

    // Load GDT
    __asm__ volatile("lgdt %0" : : "m"(g_GDTDescriptor));

    return;
}


