/*+++
    Source/KERNEL/32RTOSKRNL/CPU/ISR/ISR.h - Interrupt Service Routine (ISR) Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt service routine (ISR) implementation for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. ISR handling functions added.
REMARKS
    When compiling, include ISR.c
---*/
#ifndef ISR_H
#define ISR_H
#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#ifndef IDT_COUNT
#define IDT_COUNT 256
#endif // IDT_COUNT


struct regs {
    // Pushed by pusha
    U32 edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    // Segment registers
    U32 ds, es, fs, gs;
    // Interrupt info
    U32 int_no;
    U32 err_code;
    // CPU-pushed automatically for exceptions
    U32 eip, cs, eflags, useresp, ss;
};


typedef void (*ISRHandler)(struct regs* r);
typedef void (*IRQHandler)(struct regs* r);
typedef U0 (*ISRNUM)(U0);

void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler);
extern ISRHandler g_Handlers[IDT_COUNT];

VOID SETUP_ISR_HANDLERS(VOID);
void isr_default_handler(struct regs* r);
U0 SETUP_ISRS(U0);


#endif // ISR_H