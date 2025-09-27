/*
Unused as of now
*/
#ifndef STACK_H
#define STACK_H
#include <MEMORY.h>


static inline void RESET_STACK(void) {
    __asm__ __volatile__ (
        "movl %0, %%esp\n"
        "xor %%ebp, %%ebp\n"
        :
        : "r"(STACK_0_END)
    );
}


#endif // STACK_H