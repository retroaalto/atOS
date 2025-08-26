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

__attribute__((naked)) U0 isr0(U0);
__attribute__((naked)) U0 isr1(U0);
__attribute__((naked)) U0 isr2(U0);
__attribute__((naked)) U0 isr3(U0);
__attribute__((naked)) U0 isr4(U0);
__attribute__((naked)) U0 isr5(U0);
__attribute__((naked)) U0 isr6(U0);
__attribute__((naked)) U0 isr7(U0);
__attribute__((naked)) U0 isr8(U0);
__attribute__((naked)) U0 isr9(U0);
__attribute__((naked)) U0 isr10(U0);
__attribute__((naked)) U0 isr11(U0);
__attribute__((naked)) U0 isr12(U0);
__attribute__((naked)) U0 isr13(U0);
__attribute__((naked)) U0 isr14(U0);
__attribute__((naked)) U0 isr15(U0);
__attribute__((naked)) U0 isr16(U0);
__attribute__((naked)) U0 isr17(U0);
__attribute__((naked)) U0 isr18(U0);
__attribute__((naked)) U0 isr19(U0);
__attribute__((naked)) U0 isr20(U0);
__attribute__((naked)) U0 isr21(U0);
__attribute__((naked)) U0 isr22(U0);
__attribute__((naked)) U0 isr23(U0);
__attribute__((naked)) U0 isr24(U0);
__attribute__((naked)) U0 isr25(U0);
__attribute__((naked)) U0 isr26(U0);
__attribute__((naked)) U0 isr27(U0);
__attribute__((naked)) U0 isr28(U0);
__attribute__((naked)) U0 isr29(U0);
__attribute__((naked)) U0 isr30(U0);
__attribute__((naked)) U0 isr31(U0);
__attribute__((naked)) U0 isr32(U0);
__attribute__((naked)) U0 isr33(U0);
__attribute__((naked)) U0 isr34(U0);
__attribute__((naked)) U0 isr35(U0);
__attribute__((naked)) U0 isr36(U0);
__attribute__((naked)) U0 isr37(U0);
__attribute__((naked)) U0 isr38(U0);
__attribute__((naked)) U0 isr39(U0);
__attribute__((naked)) U0 isr40(U0);
__attribute__((naked)) U0 isr41(U0);
__attribute__((naked)) U0 isr42(U0);
__attribute__((naked)) U0 isr43(U0);
__attribute__((naked)) U0 isr44(U0);
__attribute__((naked)) U0 isr45(U0);
__attribute__((naked)) U0 isr46(U0);
__attribute__((naked)) U0 isr47(U0);
__attribute__((naked)) U0 isr48(U0);
__attribute__((naked)) U0 isr49(U0);
__attribute__((naked)) U0 isr50(U0);
__attribute__((naked)) U0 isr51(U0);

#endif // INTERRUPTS_H