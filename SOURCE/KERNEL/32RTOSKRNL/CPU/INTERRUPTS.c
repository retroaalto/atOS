#include "./INTERRUPTS.h"

// Common ISR handler wrapper
__attribute__((naked))
void isr_common32(void) {
    __asm__ volatile (
        "pusha\n\t"               // Save general registers
        "push %ds\n\t"            // Save segment registers
        "push %es\n\t"
        "push %fs\n\t"
        "push %gs\n\t"

        "mov $0x10, %ax\n\t"      // Kernel data segment
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %fs\n\t"
        "mov %ax, %gs\n\t"

        "push %esp\n\t"           // Push pointer to regs struct
        "call isr_default_handler\n\t"
        "add $4, %esp\n\t"        // Clean up stack

        "pop %gs\n\t"             // Restore segments
        "pop %fs\n\t"
        "pop %es\n\t"
        "pop %ds\n\t"

        "popa\n\t"                // Restore general registers
        "iret\n\t"
    );
}


// Macro for ISRs without error code
// ISR without CPU error code
#define ISR_NOERRORCODE(num)                   \
__attribute__((naked)) void isr##num(void) {  \
    __asm__ volatile (                         \
        "cli\n\t"                              \
        "push $0\n\t"                          /* fake error code */ \
        "push $" #num "\n\t"                   /* int_no */ \
        "jmp isr_common32\n\t"                 \
    );                                         \
}

// ISR with CPU error code
#define ISR_ERRORCODE(num)                     \
__attribute__((naked)) void isr##num(void) {  \
    __asm__ volatile (                         \
        "cli\n\t"                              \
        "push $" #num "\n\t"                   /* int_no */ \
        "jmp isr_common32\n\t"                 \
    );                                         \
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

// IRQs (32–47)
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

// All remaining vectors
ISR_NOERRORCODE(48)
ISR_NOERRORCODE(49)
ISR_NOERRORCODE(50)
ISR_NOERRORCODE(51)
