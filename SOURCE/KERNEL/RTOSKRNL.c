#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include <RTOSKRNL.h>
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

__attribute__((noreturn))
void rtos_kernel(U0) {
    CLI;
    // These are called here, to fix address issues and to set up new values
    GDT_INIT();
    IDT_INIT();
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    disable_fpu();
    panic_if(!vesa_check(), PANIC_TEXT("Failed to initialize VESA"), PANIC_INITIALIZATION_FAILED);
    
    panic_if(!vbe_check(), PANIC_TEXT("Failed to initialize VBE"), PANIC_INITIALIZATION_FAILED);
    
    
    panic_if(!E820_INIT(), PANIC_TEXT("Failed to initialize E820 memory map!"), PANIC_INITIALIZATION_FAILED);
    panic_if(!PAGEFRAME_INIT(), PANIC_TEXT("Failed to initialize page frame! Possibly not enough memory."), PANIC_INITIALIZATION_FAILED);
    panic_if(!KHEAP_INIT(KHEAP_MIN_SIZE_PAGES), PANIC_TEXT("Failed to initialize kheap. Not enough memory or pageframe issue."), PANIC_INITIALIZATION_FAILED);    
    // For some reason, PAGEFRAME or something?
    //   corrupts or zeroes the E820 entries, so we re-init it here
    panic_if(!REINIT_E820(), PANIC_TEXT("Failed to re-initialize E820!"), PANIC_INITIALIZATION_FAILED);
    panic_if(!PAGING_INIT(), PANIC_TEXT("Failed to initialize paging!"), PANIC_INITIALIZATION_FAILED);
    


    
    panic_if(!PS2_KEYBOARD_INIT(), PANIC_TEXT("Failed to initialize PS2 keyboard"), PANIC_INITIALIZATION_FAILED);
    init_multitasking();
    
    VBE_DRAW_STRING(10, 10, "atOS-RT v0.1", VBE_WHITE, VBE_BLACK);
    VBE_UPDATE_VRAM();

    STI;
    
    LOAD_AND_RUN_KERNEL_SHELL();
    RTOSKRNL_LOOP();
    system_halt(); // should never reach here
    HLT; // just in case
}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}