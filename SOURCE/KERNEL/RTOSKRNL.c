#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include "./32RTOSKRNL/RTOSKRNL.h"
/*
TODO:
    Keyboard driver
    E820
    HDD driver
    IRQ/ISR handler function re-definitions 
    Syscalls
    Shell ATOSH
    Speaker driver
    Process Management
    Multitasking?
    Inter-Process Communication

    bitmap atmp
    shell atosh
    shell lang batsh
*/


U0 itoa(U32 value, U8 *buffer, U32 base) {
    U32 i = 0;
    do {
        U32 digit = value % base;
        buffer[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
        value /= base;
    } while (value > 0);
    buffer[i] = '\0';

    // Reverse the string
    for (U32 j = 0; j < i / 2; j++) {
        U8 temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

__attribute__((noreturn))
void rtos_kernel(PTR_LIST *ptr_list) {
    HLT;
}

__attribute__((noreturn, section(".text")))
void _start(PTR_LIST *ptr_list) {
    rtos_kernel(ptr_list);
    HLT;
}