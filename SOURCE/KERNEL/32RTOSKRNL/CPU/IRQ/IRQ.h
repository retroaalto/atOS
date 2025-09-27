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
#include "../../../../STD/TYPEDEF.h"
#include "../ISR/ISR.h"


U0 IRQ_INIT(U0);

#endif // IRQ_H
