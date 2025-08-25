// isr_stubs.c
// 32-bit ISR stubs using GCC inline assembly

#include "./INTERRUPTS.h"

// Common ISR handler wrapper
__attribute__((naked)) void isr_common32(void) {
    __asm__ volatile (
        "pusha\n\t"            // save registers
        "movl %esp, %edi\n\t"  // pass pointer to stack in EDI
        "call isr_default_handler\n\t"
        "popa\n\t"             // restore registers
        "ret\n\t"
    );
}

// Macro for ISRs without error code
#define ISR_NOERRORCODE(num)                   \
__attribute__((naked)) void isr##num(void) {   \
    __asm__ volatile (                         \
        "cli\n\t"                              \
        "pushl $0\n\t"                         \
        "pushl $" #num "\n\t"                  \
        "call isr_common32\n\t"                \
        "add $8, %esp\n\t"                     \
        "iret\n\t"                             \
    );                                         \
}

// Macro for ISRs with error code
#define ISR_ERRORCODE(num)                     \
__attribute__((naked)) void isr##num(void) {   \
    __asm__ volatile (                         \
        "cli\n\t"                              \
        "pushl $" #num "\n\t"                  \
        "call isr_common32\n\t"                \
        "add $4, %esp\n\t"                     \
        "iret\n\t"                             \
    );                                         \
}

// ----------------------------------------------------
// Generate stubs
// ----------------------------------------------------
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
ISR_ERRORCODE(21)
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
ISR_NOERRORCODE(32)
ISR_NOERRORCODE(33)
ISR_NOERRORCODE(34)
ISR_NOERRORCODE(35)
ISR_NOERRORCODE(36)
ISR_NOERRORCODE(37)
ISR_NOERRORCODE(38)
ISR_NOERRORCODE(39)
ISR_NOERRORCODE(40)
ISR_NOERRORCODE(41)
ISR_NOERRORCODE(42)
ISR_NOERRORCODE(43)
ISR_NOERRORCODE(44)
ISR_NOERRORCODE(45)
ISR_NOERRORCODE(46)
ISR_NOERRORCODE(47)
ISR_NOERRORCODE(48)
ISR_NOERRORCODE(49)
ISR_NOERRORCODE(50)
