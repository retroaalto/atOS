/*+++
    Source/KERNEL/32RTOSKRNL/CPU/ISR/ISR.h - Interrupt Service Routine (ISR) Implementation

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt service routine (ISR) implementation for atOS.

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
#include "../../../../STD/TYPEDEF.h"

#define IDT_COUNT 256

typedef struct regs {
    U32 edi;
    U32 esi;
    U32 ebp;
    U32 esp;   /* original ESP pushed by pusha */
    U32 ebx;
    U32 edx;
    U32 ecx;
    U32 eax;

    U32 eip;
    U32 cs;
    U32 eflags;
} regs;


// void tss_init(void);
// #define DF_STACK_SIZE 0x4000
// extern U8 df_stack[DF_STACK_SIZE];

typedef U0 (*ISRHandler)(I32 num, U32 errcode);

void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler);
VOID SETUP_ISR_HANDLERS(VOID);
U0 SETUP_ISRS(U0);
ISRHandler *ISR_GET_PTR(void);

#endif // ISR_H