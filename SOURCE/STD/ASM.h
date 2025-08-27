#ifndef STD_ASM_H
#define STD_ASM_H

#include "./ATOSMINDEF.h"

#define ASM_VOLATILE(...) \
    __asm__ volatile(__VA_ARGS__)

#define LN "\n\t"
#define ASL(line) line LN

/* 
 * Port I/O macros
 */

// Input from port
#define inb(port, var) \
    do { \
        ASM_VOLATILE("inb %1, %0" \
            : "=a"(var) \
            : "Nd"((U32)(port))); \
    } while (0)

#define inw(port, var) \
    do { \
        ASM_VOLATILE("inw %1, %0" \
            : "=a"(var) \
            : "Nd"((U32)(port))); \
    } while (0)

#define inl(port, var) \
    do { \
        ASM_VOLATILE("inl %1, %0" \
            : "=a"(var) \
            : "Nd"((U32)(port))); \
    } while (0)

// Output to port
#define outb(port, value) \
    do { \
        ASM_VOLATILE("outb %b0, %w1" \
            : : "a"((U8)(value)), "Nd"((U32)(port))); \
    } while (0)

#define outw(port, value) \
    do { \
        ASM_VOLATILE("outw %w0, %w1" \
            : : "a"((U16)(value)), "Nd"((U32)(port)) : "memory"); \
    } while (0)

#define outl(port, value) \
    do { \
        ASM_VOLATILE("outl %0, %w1" \
            : : "a"((U32)(value)), "Nd"((U32)(port)) : "memory"); \
    } while (0)

// Block I/O (string ops)
#define insw(port, buffer, count) \
    do { \
        ASM_VOLATILE("cld; rep insw" \
            : "+D"(buffer), "+c"(count) \
            : "d"(port) \
            : "memory"); \
    } while (0)

#define outsw(port, buffer, count) \
    do { \
        ASM_VOLATILE("cld; rep outsw" \
            : "+S"(buffer), "+c"(count) \
            : "d"(port) \
            : "memory"); \
    } while (0)

// Small delay (I/O wait)
#define io_wait() \
    do { \
        ASM_VOLATILE("outb %%al, $0x80" : : "a"(0)); \
    } while (0)

#endif // STD_ASM_H
