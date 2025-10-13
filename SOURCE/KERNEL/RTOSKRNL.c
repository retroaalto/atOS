#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include <RTOSKRNL.h>

__attribute__((noreturn))
void rtos_kernel(U0) {
    CLI;
    debug_vram_start(); // Start in early mode, using direct framebuffer access
    // These are called here, to fix address issues and to set up new values
    GDT_INIT();
    IDT_INIT();
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    disable_fpu();
    panic_if(!vesa_check(), PANIC_TEXT("Failed to initialize VESA"), PANIC_INITIALIZATION_FAILED);
    
    panic_if(!vbe_check(), PANIC_TEXT("Failed to initialize VBE"), PANIC_INITIALIZATION_FAILED);

    panic_if(INITIALIZE_ATAPI() == ATA_FAILED, PANIC_TEXT("Failed to initialize ATAPI interface!"), PANIC_INITIALIZATION_FAILED);
    panic_if(!E820_INIT(), PANIC_TEXT("Failed to initialize E820 memory map!"), PANIC_INITIALIZATION_FAILED);
    panic_if(!PAGEFRAME_INIT(), PANIC_TEXT("Failed to initialize page frame! Possibly not enough memory."), PANIC_INITIALIZATION_FAILED);
    panic_if(!KHEAP_INIT(KHEAP_MAX_SIZE_PAGES), PANIC_TEXT("Failed to initialize kheap. Not enough memory or pageframe issue."), PANIC_INITIALIZATION_FAILED);    
    // For some reason, PAGEFRAME or something?
    //   corrupts or zeroes the E820 entries, so we re-init it here
    panic_if(!REINIT_E820(), PANIC_TEXT("Failed to re-initialize E820!"), PANIC_INITIALIZATION_FAILED);
    panic_if(!PAGING_INIT(), PANIC_TEXT("Failed to initialize paging!"), PANIC_INITIALIZATION_FAILED);

    
    panic_if(!PS2_KEYBOARD_INIT(), PANIC_TEXT("Failed to initialize PS2 keyboard"), PANIC_INITIALIZATION_FAILED);
    
    // panic_if(!ATA_PIIX3_INIT(), PANIC_TEXT("Failed to initialize ATA DMA"), PANIC_INITIALIZATION_FAILED);

    debug_vram_end(); // End early mode, now using task framebuffers
    init_multitasking();
    STI;
    
    INIT_RTC();
    PCI_INITIALIZE();

    SET_BEEPER_FREQUENCY(440);
    START_BEEPER();
    for(;;);

    panic_if(!ATA_PIO_INIT(), PANIC_TEXT("Failed to initialize ATA PIO"), PANIC_INITIALIZATION_FAILED);

    initialize_filestructure();

    LOAD_AND_RUN_KERNEL_SHELL();
    RTOSKRNL_LOOP();
    HLT; // just in case
    NOP;
}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}