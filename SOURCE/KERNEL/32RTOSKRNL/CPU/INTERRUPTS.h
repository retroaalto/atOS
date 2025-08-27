/*+++
    Source/KERNEL/32RTOSKRNL/CPU/INTERRUPTS.h - Interrupts Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. Contains GDT, IDT, and LDT constants.

REMARKS

    When compiling include:
        GDT/GDT.c
        IDT/IDT.c
        LDT/LDT.c
        ISR/ISR.c
        INTERRUPTS.c
---*/
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "./IDT/IDT.h"
#include "./GDT/GDT.h"
#include "./IRQ/IRQ.h"
#include "./ISR/ISR.h"

/* called from assembly stub: vector, errcode, pointer-to-saved-regs */
void isr_dispatch_c(int vector, U32 errcode, regs *regs_ptr);
void irq_dispatch_c(int irq, U32 errcode, regs *regs_ptr);

__attribute__((naked)) void isr0(void);
__attribute__((naked)) void isr1(void);
__attribute__((naked)) void isr2(void);
__attribute__((naked)) void isr3(void);
__attribute__((naked)) void isr4(void);
__attribute__((naked)) void isr5(void);
__attribute__((naked)) void isr6(void);
__attribute__((naked)) void isr7(void);
__attribute__((naked)) void isr8(void);
__attribute__((naked)) void isr9(void);
__attribute__((naked)) void isr10(void);
__attribute__((naked)) void isr11(void);
__attribute__((naked)) void isr12(void);
__attribute__((naked)) void isr13(void);
__attribute__((naked)) void isr14(void);
__attribute__((naked)) void isr15(void);
__attribute__((naked)) void isr16(void);
__attribute__((naked)) void isr17(void);
__attribute__((naked)) void isr18(void);
__attribute__((naked)) void isr19(void);
__attribute__((naked)) void isr20(void);
__attribute__((naked)) void isr21(void);
__attribute__((naked)) void isr22(void);
__attribute__((naked)) void isr23(void);
__attribute__((naked)) void isr24(void);
__attribute__((naked)) void isr25(void);
__attribute__((naked)) void isr26(void);
__attribute__((naked)) void isr27(void);
__attribute__((naked)) void isr28(void);
__attribute__((naked)) void isr29(void);
__attribute__((naked)) void isr30(void);
__attribute__((naked)) void isr31(void);
void irq32(void);
void irq33(void);
void irq34(void);
void irq35(void);
void irq36(void);
void irq37(void);
void irq38(void);
void irq39(void);
void irq40(void);
void irq41(void);
void irq42(void);
void irq43(void);
void irq44(void);
void irq45(void);
void irq46(void);
void irq47(void);
__attribute__((naked)) void isr48(void);
__attribute__((naked)) void isr49(void);
__attribute__((naked)) void isr50(void);
__attribute__((naked)) void isr51(void);

#endif // INTERRUPTS_H