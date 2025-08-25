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

static GDTENTRY gdt[GDT_ENTRY_COUNT] __attribute__((aligned(8)));
static GDTDESCRIPTOR *gdtr = (GDTDESCRIPTOR *)MEM_GDT_BASE;
void gdt_set_gate(I32 num, U32 base, U32 limit, U8 access, U8 gran) {
    gdt[num].Base0    = (base & 0xFFFF);
    gdt[num].Base1 = (base >> 16) & 0xFF;
    gdt[num].Base2   = (base >> 24) & 0xFF;

    gdt[num].Limit0   = (limit & 0xFFFF);
    gdt[num].Limit1_Flags = ((limit >> 16) & 0x0F);

    gdt[num].Limit1_Flags |= (gran & 0xF0);
    gdt[num].AccessByte      = access;
}

VOID GDT_INIT(VOID) {
    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdtr.size = (sizeof(GDTENTRY) * GDT_ENTRY_COUNT) - 1;
    gdtr.offset = (U32)&gdt;

}
