/*+++
    Source/KERNEL/32RTOSKRNL/CPU/IDT/IDT.h - IDT Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit IDT definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains LDT structures and constants.

REMARKS

    Only for kernel usage.
---*/
#include "./IDT.h"
#include "../GDT/GDT.h"


#define IDT_COUNT 256
static IDTENTRY idt[IDT_COUNT] __attribute__((aligned(8)));
static IDTDESCRIPTOR idtr;

static void idt_set_gate(I32 idx, U32 handler_addr, U16 sel, U8 type_attr) {
    idt[idx].OffsetLow  = handler_addr & 0xFFFF;
    idt[idx].Selector   = sel;
    idt[idx].Zero       = 0;
    idt[idx].TypeAttr   = type_attr; // 0x8E
    idt[idx].OffsetHigh = (handler_addr >> 16) & 0xFFFF;
}

void isr_handler() {
    // Handle the interrupt
}

void isr_stub_common() {
    __asm__ volatile (
        "push %eax\n"
        "push %ecx\n"
        "push %edx\n"
        "call isr_handler\n"
        "pop %edx\n"
        "pop %ecx\n"
        "pop %eax\n"
        "iret\n"
    );
}

void IDT_INIT() {
    for (int i = 0; i < IDT_COUNT; ++i) {
        idt_set_gate(i, (U32)isr_stub_common, KCODE_SEL, 0x8E);
    }
    idtr.size = sizeof(idt) - 1;
    idtr.offset = (U32)&idt;
    __asm__ volatile("lidt %0" : : "m"(idtr));
}
