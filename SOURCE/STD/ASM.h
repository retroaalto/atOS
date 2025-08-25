#ifndef STD_ASM_H
#define STD_ASM_H

#include "./ATOSMINDEF.h"

STATIC INLINE void insw(U16 __port, void *__buffer, U32 __count) {
    ASM VOLATILE("cld; rep insw"
        : "+D"(__buffer), "+c"(__count)
        : "d"(__port)
        : "memory");
}

STATIC INLINE void outsw(U16 __port, const void *__buffer, U32 __count) {
    ASM VOLATILE("cld; rep outsw"
        : "+S"(__buffer), "+c"(__count)
        : "d"(__port)
        : "memory");
}

#define ASM_VOLATILE(x) __asm__ volatile (x)

/*+++
INx and OUTx functions
---*/

STATIC INLINE U8 inb(U16 __port) {
    U8 __ret;
    ASM VOLATILE("inb %1, %0"
        : "=a"(__ret)
        : "Nd"(__port));
    return __ret;
}
STATIC INLINE U16 inw(U16 __port) {
    U16 __ret;
    ASM VOLATILE("inw %1, %0"
        : "=a"(__ret)
        : "Nd"(__port));
    return __ret;
}
STATIC INLINE U32 inl(U16 __port) {
    U32 __ret;
    ASM VOLATILE("inl %1, %0"
        : "=a"(__ret)
        : "Nd"(__port));
    return __ret;
}

STATIC INLINE VOID outb(U16 __port, U8 __value) {
    ASM volatile ( "outb %b0, %w1" : : "a"(__value), "Nd"(__port));
}
STATIC INLINE VOID outw(U16 __port, U16 __value) {
    ASM volatile ( "outw %w0, %w1" : : "a"(__value), "Nd"(__port) : "memory");
}
STATIC INLINE VOID outl(U16 __port, U32 __value) {
    ASM volatile ( "outl %0, %w1" : : "a"(__value), "Nd"(__port) : "memory");
}

STATIC INLINE VOID io_wait(VOID){
    ASM VOLATILE ( "outb %%al, $0x80" : : "a"(0) );
}
#endif // STD_ASM_H
