#include "./INTERRUPTS.h"

// Common ISR handler wrapper
typedef void (*isr_t)(void);


// Define IRQ wrappers 32..47 (we will create generic wrappers)
// Exceptions without CPU error code
#define ISR_NOERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "pusha\n\t"                    /* save regs (EDI..EAX) */ \
        "movl %%esp, %%ecx\n\t"          /* ecx = &vector */ \
        "addl $8, %%ecx\n\t"            /* ecx = &regs (start of pusha frame) */ \
        "pushl %%ecx\n\t"               /* regs* */ \
        "pushl $0\n\t"                 /* fake error code */ \
        "pushl $" #n "\n\t"            /* vector */ \
        "call isr_dispatch_c\n\t" \
        "addl $12, %%esp\n\t"           /* pop (regs*, err, vector) */ \
        "popa\n\t" \
        "iret\n\t" ::: "memory"); \
}

// Exceptions with CPU error code (e.g., 0x0E Page Fault)
// After pusha, CPU error code is at [esp+32]
#define ISR_ERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "pusha\n\t" \
        "movl %%esp, %%ecx\n\t" \
        "addl $8, %%ecx\n\t"            /* regs* */ \
        "pushl %%ecx\n\t" \
        "pushl 32(%%esp)\n\t"           /* copy CPU error code */ \
        "pushl $" #n "\n\t"            /* vector */ \
        "call isr_dispatch_c\n\t" \
        "addl $12, %%esp\n\t" \
        "popa\n\t" \
        "iret\n\t" ::: "memory"); \
}

// IRQ wrappers (vectors 0x20..0x2F). Pass VECTOR to C.
#define IRQ_WRAPPER(vec) \
__attribute__((naked)) void irq##vec(void) { \
    asm volatile( \
        "pusha\n\t" \
        "movl %%esp, %%ecx\n\t" \
        "addl $32, %%ecx\n\t"          /* &regs struct after pusha */ \
        "pushl %%ecx\n\t"               /* regs_ptr (3rd param) */ \
        "pushl $0\n\t"                 /* errcode (2nd param) */ \
        "pushl $" #vec "\n\t"          /* vector (1st param) */ \
        "call irq_dispatch_c\n\t" \
        "addl $12, %%esp\n\t" \
        "popa\n\t" \
        "iret\n\t" ::: "memory"); \
}


// ----------------------------------------------------
// Generate stubs
// ----------------------------------------------------
// Exceptions (some with error code)
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
ISR_NOERRORCODE(49)
ISR_NOERRORCODE(50)
ISR_NOERRORCODE(51)