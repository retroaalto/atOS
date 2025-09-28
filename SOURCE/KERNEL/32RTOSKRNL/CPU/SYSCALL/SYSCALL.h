#ifndef SYSCALL_H
#define SYSCALL_H

#ifdef KERNEL_ENTRY
#warning "Not to be used inside KERNEL.c"
#endif // KERNEL_ENTRY

#ifndef KERNEL_ENTRY
#include <STD/TYPEDEF.h>

#define SYSCALL_VECTOR 0x80

typedef U32 (*SYSCALL_HANDLER)(U32, U32, U32, U32, U32);

typedef enum {
    SYSCALL_EXIT = 0,
    SYSCALL_PUTS,
    SYSCALL_UPDATE_VRAM,
    SYSCALL_DRAW_ELLIPSE,

    SYSCALL_MAX,
} SYSCALL_NUM;

/*+++ 
Kernel-space function prototypes 
These functions are implemented in SYSCALL.c and can be used in the kernel.
External user-space code should use the SYSCALL macros instead, located below.
---*/
U32 SYS_EXIT(U32 code, U32 unused1, U32 unused2, U32 unused3, U32 unused4);
U32 SYS_PUTS(U32 str_ptr, U32 unused1, U32 unused2, U32 unused3, U32 unused4);
U32 SYS_UPDATE_VRAM(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5);
U32 SYS_DRAW_ELLIPSE(U32 x, U32 y, U32 a, U32 b, U32 color);


/*+++ 
User-space macros 
Use these macros to invoke system calls from user-space code.
These don't require inline assembly and are easier to use.

Use interrupt 0x80 to invoke a syscall.

Syscall convention:
    EAX: syscall number
    EBX: arg1
    ECX: arg2
    EDX: arg3
    ESI: arg4
    EDI: arg5
    Return value in EAX
---*/
#define SYSCALL(num, a1, a2, a3, a4, a5) ({ \
    U32 ret; \
    asm volatile ( \
        "int $0x80" \
        : "=a"(ret) \
        : "a"(num), "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5) \
        : "memory" \
    ); \
    ret; \
})
#define EXIT_SYS() SYSCALL(SYSCALL_EXIT, 0, 0, 0, 0, 0)
#define PUTS_SYS(str) SYSCALL(SYSCALL_PUTS, (U32)(str), 0, 0, 0, 0)
#define UPDATE_VRAM_SYS() SYSCALL(SYSCALL_UPDATE_VRAM, 0, 0, 0, 0, 0)
#define DRAW_ELLIPSE_SYS(x,y,a,b,color) SYSCALL(SYSCALL_DRAW_ELLIPSE, (U32)(x), (U32)(y), (U32)(a), (U32)(b), (U32)(color))

#ifdef __RTOS__
U32 syscall_dispatcher(U32 num, U32 a1, U32 a2, U32 a3, U32 a4, U32 a5);
__attribute__((naked)) void isr_syscall(void);  
#endif // __RTOS__

#endif // KERNEL_ENTRY
#endif // SYSCALL_H