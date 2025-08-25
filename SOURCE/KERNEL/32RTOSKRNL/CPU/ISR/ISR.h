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
    U32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    U32 ds, es, fs, gs, ss;
    U32 eip, cs, eflags;
    U32 int_no;
};

typedef void (*ISRHandler)(struct regs* r);
typedef void (*IRQHandler)(struct regs* r);

void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler);
extern ISRHandler g_Handlers[IDT_COUNT];

VOID SETUP_ISR_HANDLERS(VOID);
void isr_default_handler(struct regs* r);
U0 SETUP_ISRS();

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


#endif // ISR_H