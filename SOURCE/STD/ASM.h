#ifndef STD_ASM_H
#define STD_ASM_H

#include "./ATOSMINDEF.h"

static inline void insw(U16 __port, void *__buffer, U32 __count) {
    ASM VOLATILE("cld; rep insw"
        : "+D"(__buffer), "+c"(__count)
        : "d"(__port)
        : "memory");
}

static inline void outsw(U16 __port, const void *__buffer, U32 __count) {
    ASM VOLATILE("cld; rep outsw"
        : "+S"(__buffer), "+c"(__count)
        : "d"(__port)
        : "memory");
}

#define ASM_VOLATILE(...) \
    __asm__ volatile(__VA_ARGS__)

/*+++
INx and OUTx functions
---*/

// Disable warning -Wcomment

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wcomments"

// #define inb(port) \
//     U8 __ret; \
//     do { \
//         ASM_VOLATILE("inb %1, %0" \
//             : "=a"(__ret) \
//             : "Nd"((U32)port)); \
//     } while (0)

// #define inw(port) \
//     U16 __ret; \
//     do { \
//         ASM_VOLATILE("inw %1, %0" \
//             : "=a"(__ret) \
//             : "Nd"((U32)port)); \
//     } while (0)

// #define inl(port) \
//     U32 __ret; \
//     do { \
//         ASM_VOLATILE("inl %1, %0" \
//             : "=a"(__ret) \
//             : "Nd"((U32)port)); \
//     } while (0)

// #define outb(port, value) \
//     do { \
//         ASM_VOLATILE("outb %b0, %w1" : : "a"((U8)value), "Nd"((U32)port)); \
//     } while (0)

// #define outw(port, value) \
//     do { \
//         ASM_VOLATILE("outw %w0, %w1" : : "a"((U16)value), "Nd"((U32)port) : "memory"); \
//     } while (0)
// #define outl(port, value) \
//     do { \
//         ASM_VOLATILE("outl %0, %w1" : : "a"((U32)value), "Nd"((U32)port) : "memory"); \
//     } while (0)

static inline void outb(U16 __port, U8 __value) {
    ASM_VOLATILE("outb %b0, %w1" : : "a"(__value), "Nd"((U32)__port));
}

static inline void outw(U16 __port, U16 __value) {
    ASM_VOLATILE("outw %w0, %w1" : : "a"(__value), "Nd"((U32)__port));
}

static inline void outl(U16 __port, U32 __value) {
    ASM_VOLATILE("outl %0, %w1" : : "a"(__value), "Nd"((U32)__port));
}
static inline U8 inb(U16 __port) {
    U8 __ret;
    ASM_VOLATILE("inb %1, %0"
        : "=a"(__ret)
        : "Nd"((U32)__port));
    return __ret;
}

static inline U16 inw(U16 __port) {
    U16 __ret;
    ASM_VOLATILE("inw %1, %0"
        : "=a"(__ret)
        : "Nd"((U32)__port));
    return __ret;
}

static inline U32 inl(U16 __port) {
    U32 __ret;
    ASM_VOLATILE("inl %1, %0"
        : "=a"(__ret)
        : "Nd"((U32)__port));
    return __ret;
}

// #define io_wait() \
//     do { \
//         __asm__ volatile("outb %%al, $0x80" : : "a"(0)); \
//     } while (0)

#pragma GCC diagnostic pop

static inline void io_wait(void) {
    ASM_VOLATILE("outb %%al, $0x80" : : "a"(0));
}

#endif // STD_ASM_H
