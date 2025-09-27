/*+++
    Source/KERNEL/32RTOSKRNL/CPU/GDT/GDT.c - GDT Implementation

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit GDT and IDT implementations for atOS.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains GDT structures and constants.

REMARKS

    Only for kernel usage.

---*/
#include "GDT.h"
#include "../../../../STD/ASM.h"



#define GDT_ENTRY_COUNT 3

#define READABLE_RNG0_KRNL 0x9A
#define WRITABLE_RNG0_KRNL 0x92
#define GRANULARITY 0xCF

static GDTENTRY g_GDT[GDT_ENTRY_COUNT] = { 0 };
static GDTDESCRIPTOR g_GDTDescriptor = { 0 };

GDTDESCRIPTOR *GDT_GET_PTR(void) {
    return (U0 *)&g_GDTDescriptor;
}

void gdt_set_gate(U32 num, U32 base, U32 limit, U8 access, U8 gran) {
    g_GDT[num].base0       = base & 0xFFFF;
    g_GDT[num].base1       = (base >> 16) & 0xFF;
    g_GDT[num].base2       = (base >> 24) & 0xFF;
    g_GDT[num].limit0      = limit & 0xFFFF;
    g_GDT[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    g_GDT[num].access      = access;
}

U0 GDT_INIT(U0) {
    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);                  
    // Kernel code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, READABLE_RNG0_KRNL, GRANULARITY); 
    // Kernel data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, WRITABLE_RNG0_KRNL, GRANULARITY); 

    g_GDTDescriptor.limit = GDT_ENTRY_COUNT * sizeof(GDTENTRY) - 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
    g_GDTDescriptor.base  = g_GDT;
#pragma GCC diagnostic pop

    __asm__ volatile("lgdt %0" : : "m"(g_GDTDescriptor));
}


