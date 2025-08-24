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

// ISR stub macro
#define ISR_STUB_DEF(n) \
__attribute__((naked)) void isr_##n(void); \

#define ISR_STUB(n) \
__attribute__((naked)) void isr_##n(void) { \
    __asm__ volatile ( \
        "cli\n\t" \
        "push %%ds\n\t" \
        "push %%es\n\t" \
        "push %%fs\n\t" \
        "push %%gs\n\t" \
        "push %%eax\n\t" \
        "push %%ecx\n\t" \
        "push %%edx\n\t" \
        "push %%ebx\n\t" \
        "push %%esp\n\t" \
        "push %%ebp\n\t" \
        "push %%esi\n\t" \
        "push %%edi\n\t" \
        "movl %0, %%eax\n\t" \
        "push %%eax\n\t" \
        "leal 4(%%esp), %%eax\n\t" \
        "push %%eax\n\t" \
        "call isr_handler\n\t" \
        "addl $8, %%esp\n\t" \
        "pop %%edi\n\t" \
        "pop %%esi\n\t" \
        "pop %%ebp\n\t" \
        "pop %%esp\n\t" \
        "pop %%ebx\n\t" \
        "pop %%edx\n\t" \
        "pop %%ecx\n\t" \
        "pop %%eax\n\t" \
        "pop %%gs\n\t" \
        "pop %%fs\n\t" \
        "pop %%es\n\t" \
        "pop %%ds\n\t" \
        "iret\n\t" \
        : : "i"(n) \
    ); \
}
struct regs {
    U32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    U32 ds, es, fs, gs, ss;
    U32 eip, cs, eflags;
    U32 int_no;
};
typedef void (*ISRHandler)(struct regs* r);
#define IDT_PTR ((IDTENTRY*)IDT_MEM_BASE)
void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler);
extern ISRHandler g_Handlers[IDT_COUNT];
VOID SETUP_ISR_HANDLERS(VOID);
void isr_default_handler(struct regs* r);
void isr0_handler(struct regs* r);
ISR_STUB_DEF(0)
ISR_STUB_DEF(1)
ISR_STUB_DEF(2)
ISR_STUB_DEF(3)
#endif // ISR_H