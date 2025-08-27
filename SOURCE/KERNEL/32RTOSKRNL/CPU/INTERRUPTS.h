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
void tss_init(void);
void isr0(void);
void isr1(void);
void isr2(void);
void isr3(void);
void isr4(void);
void isr5(void);
void isr6(void);
void isr7(void);
void isr8(void);
void isr9(void);
void isr10(void);
void isr11(void);
void isr12(void);
void isr13(void);
void isr14(void);
void isr15(void);
void isr16(void);
void isr17(void);
void isr18(void);
void isr19(void);
void isr20(void);
void isr21(void);
void isr22(void);
void isr23(void);
void isr24(void);
void isr25(void);
void isr26(void);
void isr27(void);
void isr28(void);
void isr29(void);
void isr30(void);
void isr31(void);
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
void isr48(void);
void isr49(void);
void isr50(void);
void isr51(void);

#endif // INTERRUPTS_H