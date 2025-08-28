#define RTOS_KERNEL
#include "./32RTOSKRNL/RTOSKRNL.h"
/*
TODO:
    
    E820
    HDD driver
    Speaker driver
    Timer driver
    Keyboard driver
    Syscalls
    Shell
    Process Management
    Multitasking?
    Inter-Process Communication
    
*/

__attribute__((noreturn))
void rtos_kernel(U0) {
    VBE_DRAW_STRING(0, 0, "HELLOOOO! FROM MAIN RTOS KERNEL!!", VBE_AQUA, VBE_RED);
    VBE_STOP_DRAWING();
    HLT;
}

__attribute__((noreturn, section(".text")))
void _start(void) {
    rtos_kernel();
}