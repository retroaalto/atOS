#include "./INTERRUPTS/INTERRUPTS.h"

// Common ISR handler wrapper
typedef void (*isr_t)(void);

// Adjusted ISR macros to pass correct regs pointer
// Exceptions without CPU error code
#define ISR_NOERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "movl %%esp, %%eax\n\t"   /* eax = current esp */ \
        "addl $28, %%eax\n\t"     /* adjust to point to EAX (first pusha register) */ \
        "pushl %%eax\n\t"         /* push regs* argument */ \
        "pushl $0\n\t"            /* fake errcode */ \
        "pushl $" #n "\n\t"       /* vector */ \
        "call isr_dispatch_c\n\t" \
        "addl $12, %%esp\n\t"     /* pop args */ \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" ::: "eax", "memory"); \
}

// Exceptions with CPU error code (e.g., page fault)
#define ISR_ERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "movl 32(%%esp), %%edx\n\t" /* edx = CPU error code BEFORE pushing more */ \
        "movl %%esp, %%eax\n\t" \
        "addl $28, %%eax\n\t"      /* adjust to point to EAX */ \
        "pushl %%eax\n\t"          /* push regs* */ \
        "pushl %%edx\n\t"          /* push CPU error code */ \
        "pushl $" #n "\n\t"        /* vector */ \
        "call isr_dispatch_c\n\t" \
        "addl $12, %%esp\n\t" \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" ::: "eax","edx","memory"); \
}

// IRQ wrappers (vectors 0x20..0x2F)
#define IRQ_WRAPPER(vec) \
__attribute__((naked)) void irq##vec(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "movl %%esp, %%eax\n\t" \
        "addl $28, %%eax\n\t"     /* adjust to start of pusha */ \
        "pushl %%eax\n\t"         /* push regs* */ \
        "pushl $0\n\t"            /* fake errcode */ \
        "pushl $" #vec "\n\t"     /* vector */ \
        "call irq_dispatch_c\n\t" \
        "addl $12, %%esp\n\t" \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" ::: "eax","memory"); \
}

// ----------------------------------------------------
// Generate stubs
// ----------------------------------------------------
// Exceptions (some with error code)

#ifdef __RTOS__
#define ISR_NOERRORCODE ISR_ERRORCODE
#endif // __RTOS__
ISR_NOERRORCODE(0)
ISR_NOERRORCODE(1)
ISR_NOERRORCODE(2)
ISR_NOERRORCODE(3)
ISR_NOERRORCODE(4)
ISR_NOERRORCODE(5)
ISR_NOERRORCODE(6)
ISR_NOERRORCODE(7)
ISR_ERRORCODE(8)
ISR_NOERRORCODE(9)
ISR_ERRORCODE(10)
ISR_ERRORCODE(11)
ISR_ERRORCODE(12)
ISR_ERRORCODE(13)
ISR_ERRORCODE(14)
ISR_NOERRORCODE(15)
ISR_NOERRORCODE(16)
ISR_ERRORCODE(17)
ISR_NOERRORCODE(18)
ISR_NOERRORCODE(19)
ISR_NOERRORCODE(20)
ISR_NOERRORCODE(21)
ISR_NOERRORCODE(22)
ISR_NOERRORCODE(23)
ISR_NOERRORCODE(24)
ISR_NOERRORCODE(25)
ISR_NOERRORCODE(26)
ISR_NOERRORCODE(27)
ISR_NOERRORCODE(28)
ISR_NOERRORCODE(29)
ISR_NOERRORCODE(30)
ISR_NOERRORCODE(31)

// IRQs (32–47)
IRQ_WRAPPER(32)
IRQ_WRAPPER(33)
IRQ_WRAPPER(34)
IRQ_WRAPPER(35)
IRQ_WRAPPER(36)
IRQ_WRAPPER(37)
IRQ_WRAPPER(38)
IRQ_WRAPPER(39)
IRQ_WRAPPER(40)
IRQ_WRAPPER(41)
IRQ_WRAPPER(42)
IRQ_WRAPPER(43)
IRQ_WRAPPER(44)
IRQ_WRAPPER(45)
IRQ_WRAPPER(46)
IRQ_WRAPPER(47)

// All remaining vectors
ISR_NOERRORCODE(48)
