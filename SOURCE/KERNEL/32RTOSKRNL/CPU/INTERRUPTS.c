#include "./INTERRUPTS.h"

// Common ISR handler wrapper
typedef void (*isr_t)(void);


// Define IRQ wrappers 32..47 (we will create generic wrappers)
#define IRQ_WRAPPER(n) \
__attribute__((naked)) void irq##n(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "pushl $" #n "\n\t" /* vector */ \
        "pushl $0\n\t"       /* fake error code for uniformity */ \
        "call irq_common_stub\n\t" \
        "addl $8, %%esp\n\t" \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" \
        : : : "memory"); \
}

// ISR without error code
#define ISR_NOERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "pushl $" #n "\n\t" /* vector */ \
        "pushl $0\n\t"       /* fake error code */ \
        "call isr_common_stub\n\t" \
        "addl $8, %%esp\n\t" \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" \
        : : : "memory"); \
}

// ISR with CPU error code
#define ISR_ERRORCODE(n) \
__attribute__((naked)) void isr##n(void) { \
    asm volatile( \
        "cli\n\t" \
        "pusha\n\t" \
        "pushl $" #n "\n\t" /* vector */ \
        "call isr_common_stub\n\t" \
        "addl $8, %%esp\n\t" \
        "popa\n\t" \
        "sti\n\t" \
        "iret\n\t" \
        : : : "memory"); \
}

__attribute__((naked)) void isr_common_stub(void) {
    asm volatile(
        "pushl %ebp\n\t"
        "movl %esp, %ebp\n\t"
        // Arguments for C handler: vector = [ebp+36], error_code = [ebp+32], regs = [ebp+16]
        "leal 16(%ebp), %ecx\n\t"   /* regs_t* */ 
        "movl 32(%ebp), %edx\n\t"   /* error code */ 
        "movl 36(%ebp), %eax\n\t"   /* vector */ 
        "pushl %ecx\n\t"
        "pushl %edx\n\t"
        "pushl %eax\n\t"
        "call isr_dispatch_c\n\t"
        "addl $12, %esp\n\t"
        "popl %ebp\n\t"
        "ret\n\t"
    );
}

// IRQ common stub
__attribute__((naked)) void irq_common_stub(void) {
    asm volatile(
        "pushl %ebp\n\t"
        "movl %esp, %ebp\n\t"
        "leal 16(%ebp), %ecx\n\t" /* regs_t* */
        "movl 32(%ebp), %eax\n\t" /* vector/IRQ number */
        "pushl %ecx\n\t"
        "pushl $0\n\t"           /* fake error code */ 
        "pushl %eax\n\t"
        "call irq_dispatch_c\n\t"
        "addl $12, %esp\n\t"
        "popl %ebp\n\t"
        "ret\n\t"
    );
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

__attribute__((naked)) void isr8(void) {
    asm volatile(
        "cli\n\t"
        // "movl %[stack_top], %%esp\n\t"
        // "pusha\n\t"
        // "pushl $8\n\t"
        // "call isr_common_stub\n\t"
        "hlt\n\t"
        // :
        // : [stack_top] "r" (&df_stack[DF_STACK_SIZE])
        // : "memory"
    );
}


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
