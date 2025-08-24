/*+++
    Source/KERNEL/32RTOSKRNL/CPU/IRQ/IRQ.h - Interrupt Request (IRQ) Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt request (IRQ) definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. IRQ remapping functions added.
REMARKS

    When compiling include:
        IRQ.c
---*/
#ifndef IRQ_H
#define IRQ_H

#include "../../../../STD/ASM.h"
#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"
#include "../ISR/ISR.h"

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01
void IRQ_INIT();
void irq_default_handler(struct regs* r);

#endif // IRQ_H
