#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include <RTOSKRNL.h>
#include <CPU/SYSCALL/SYSCALL.h>
/*
TODO:
    Test via Shell:
        E820 & Paging
        Process Management
        Multitasking
        Inter-Process Communication

    Shell ATOSH
    HDD driver
    Syscalls
    Speaker driver

    bitmap atmp
    shell atosh
    shell lang batsh
*/

#define INC_ROW(row) (row += VBE_CHAR_HEIGHT + 2)
#define DRAW_STRING(text, color) VBE_DRAW_STRING(0, row, text, color, VBE_BLACK); INC_ROW(row); VBE_UPDATE_VRAM();


__attribute__((noreturn))
void rtos_kernel(U0) {
    U32 row = 0;
    CLI;
    // These are called here, to fix address issues and to set up new values
    GDT_INIT();
    IDT_INIT();
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    
    PIT_INIT();
    vesa_check();
    vbe_check();

    if(!PAGEFRAME_INIT()) {
        DRAW_STRING("Failed to initialize page frame. Possibly not enough memory.", VBE_RED);
        HLT;
    }
    INIT_PAGING();

    kernel_heap_init();
    // user_heap_init(); // TODO: This mess

    if(!PS2_KEYBOARD_INIT()) {
        DRAW_STRING("Failed to initialize PS2 keyboard", VBE_RED);
        HLT;
    }

    STI;

    LOAD_AND_RUN_KERNEL_SHELL();

}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}