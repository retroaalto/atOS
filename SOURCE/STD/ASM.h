#ifndef STD_ASM_H
#define STD_ASM_H

#include "./TYPEDEF.h"

#define ASM_VOLATILE(...) \
    __asm__ volatile(__VA_ARGS__)
#define HLT for(;;) ASM_VOLATILE("cli; hlt")

#define LN "\n\t"
#define ASL(line) line LN

#define HALT ASM_VOLATILE("hlt")
#define NOP ASM_VOLATILE("nop")
#define CLI ASM_VOLATILE("cli")
#define STI ASM_VOLATILE("sti")

/* 
 * Port I/O macros (legacy, kept for compatibility)
 */


// Single, canonical helpers (drop the macro variants)

static inline void _io_wait(void) {
    unsigned char dummy;
    asm volatile ("inb $0x80, %0"
                  : "=a"(dummy)
                  : 
                  : "memory");
}

static inline unsigned char _inb(unsigned short port) {
    unsigned char v;
    asm volatile ("inb %1, %0"
                  : "=a"(v)
                  : "Nd"(port)
                  : "memory");
    return v;
}

static inline void _outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1"
                  :
                  : "a"(val), "Nd"(port)
                  : "memory");
}

static inline unsigned short _inw(unsigned short port) {
    unsigned short v;
    asm volatile ("inw %1, %0"
                  : "=a"(v)
                  : "Nd"(port)
                  : "memory");
    return v;
}

static inline void _outw(unsigned short port, unsigned short val) {
    asm volatile ("outw %0, %1"
                  :
                  : "a"(val), "Nd"(port)
                  : "memory");
}

static inline unsigned int _inl(unsigned short port) {
    unsigned int v;
    asm volatile ("inl %1, %0"
                  : "=a"(v)
                  : "Nd"(port)
                  : "memory");
    return v;
}

static inline void _outl(unsigned short port, unsigned int val) {
    asm volatile ("outl %0, %1"
                  :
                  : "a"(val), "Nd"(port)
                  : "memory");
}

// Read `count` 16-bit words from port into buffer
static inline void _insw(unsigned short port, void *buffer, unsigned int count) {
    asm volatile ("cld; rep insw"
                  : "+D"(buffer), "+c"(count)
                  : "d"(port)
                  : "memory");
}

// Write `count` 16-bit words from buffer to port
static inline void _outsw(unsigned short port, const void *buffer, unsigned int count) {
    asm volatile ("cld; rep outsw"
                  : "+S"(buffer), "+c"(count)
                  : "d"(port)
                  : "memory");
}


/* 
 * Inline functions version (returns values or takes parameters)
 */

static inline U8 _inb(U16 port) {
    U8 ret;
    ASM_VOLATILE("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline U16 _inw(U16 port) {
    U16 ret;
    ASM_VOLATILE("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline U32 _inl(U16 port) {
    U32 ret;
    ASM_VOLATILE("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void _outb(U16 port, U8 val) {
    ASM_VOLATILE("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

static inline void _outw(U16 port, U16 val) {
    ASM_VOLATILE("outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void _outl(U16 port, U32 val) {
    ASM_VOLATILE("outl %0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void _insw(U16 port, void* buffer, U32 count) {
    ASM_VOLATILE("cld; rep insw"
                 : "+D"(buffer), "+c"(count)
                 : "d"(port)
                 : "memory");
}

static inline void _outsw(U16 port, void* buffer, U32 count) {
    ASM_VOLATILE("cld; rep outsw"
                 : "+S"(buffer), "+c"(count)
                 : "d"(port)
                 : "memory");
}

static inline void _io_wait() {
    ASM_VOLATILE("outb %%al, $0x80" : : "a"(0));
}

#endif // STD_ASM_H
