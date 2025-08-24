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
#include "./GDT.h"

static GDTENTRY gdt[GDT_ENTRY_COUNT] __attribute__((aligned(16)));
static GDTDESCRIPTOR gdtr;

static inline void gdt_set_gate(I32 num, U32 base, U32 limit, U8 access, U8 gran) {
    gdt[num].Base0 = (U16)(base & 0xFFFF);
    gdt[num].Base1 = (U8)((base >> 16) & 0xFF);
    gdt[num].Base2 = (U8)((base >> 24) & 0xFF);

    gdt[num].Limit0 = (U16)(limit & 0xFFFF);
    gdt[num].Limit1_Flags = (U8)((limit >> 16) & 0x0F);
    gdt[num].Limit1_Flags |= (gran & 0xF0); // flags live in high nibble
    gdt[num].AccessByte = (U8)access;
}

VOID GDT_INIT(VOID) {
    // Null, Kernel code, Kernel data (flat 32-bit: limit = 0xFFFFF + G=1)
    gdt_set_gate(GDT_IDX_NULL,  0,       0,        0x00,       0x00);       // null
    gdt_set_gate(GDT_IDX_KCODE, 0, 0xFFFFF, ACC_KCODE, GRAN_32_4K);         // kernel code
    gdt_set_gate(GDT_IDX_KDATA, 0, 0xFFFFF, ACC_KDATA, GRAN_32_4K);         // kernel data

    gdtr.size = (sizeof(gdt) - 1);
    gdtr.offset = (U32)&gdt;

    __asm__ volatile(
        "cli\n\t"                    // ensure interrupts off during switch
        "lgdt %0\n\t"                // load new GDT
        "mov %1, %%ax\n\t"           // load data selectors
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "ljmp %2, $1f\n\t"           // far jump to reload CS
        "1:\n\t"
        : : "m"(gdtr), "i"(KDATA_SEL), "i"(KCODE_SEL) : "ax", "memory"
    );

}
