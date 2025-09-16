/*+++
    Source/KERNEL/32RTOSKRNL/CPU/IRQ/IRQ.h - Interrupt Request (IRQ) Definitions

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt request (IRQ) definitions for atOS.

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

#define PIC_REMAP_OFFSET 0x20
#define IRQ_LINES        16
#define IRQ_KEYBOARD     1
U0 IRQ_INIT(U0);
void pic_send_eoi(U8 irq);
void pic_remap(U8 offset1, U8 offset2);
void IRQ_SET_MASK(U8 irq);
void IRQ_CLEAR_MASK(U8 irq);
void IRQ_REGISTER_HANDLER(U8 irq, IRQHandler handler);
#endif // IRQ_H
