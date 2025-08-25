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
---*/
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "./IDT/IDT.h"
#include "./GDT/GDT.h"
#include "./IRQ/IRQ.h"

void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();
void isr32();
void isr33();
void isr34();
void isr35();
void isr36();
void isr37();
void isr38();
void isr39();
void isr40();
void isr41();
void isr42();
void isr43();
void isr44();
void isr45();
void isr46();
void isr47();
void isr48();
void isr49();
void isr50();

#endif // INTERRUPTS_H