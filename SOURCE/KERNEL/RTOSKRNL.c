#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include <RTOSKRNL.h>
/*
TODO:
    E820 & Paging
    Keyboard driver
    HDD driver
    Speaker driver
    Syscalls
    Shell ATOSH
    Process Management
    Multitasking?
    Inter-Process Communication

    bitmap atmp
    shell atosh
    shell lang batsh
*/

#define INC_ROW(row) (row += VBE_CHAR_HEIGHT + 2)
#define DRAW_STRING(text, color) VBE_DRAW_STRING(0, row, text, color, VBE_BLACK); INC_ROW(row); VBE_UPDATE_VRAM();


__attribute__((noreturn))
void rtos_kernel(U0) {
    VBE_DRAW_LINE(0, 0, 1024, 0, VBE_RED); 
    VBE_UPDATE_VRAM();
    U32 row = 0;
    CLI;
    // Recalled here, to fix address issues
    GDT_INIT();
    IDT_INIT();
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    
    PIT_INIT();
    vesa_check();
    vbe_check();

    if(!PS2_KEYBOARD_INIT()) {
        DRAW_STRING("Failed to initialize PS2 keyboard", VBE_RED);
        HLT;
    }
    if(!PAGEFRAME_INIT()) {
        DRAW_STRING("Failed to initialize page frame. Possibly not enough memory.", VBE_RED);
        HLT;
    }
    // INIT_PAGING();

    STI;
    

    U8 buf[100];
    U32 *pit_ticks = PIT_GET_TICKS_PTR();
    U32 i = 0;
    for(; i < 10; i++) {
        ITOA(*pit_ticks, buf, 16);
        VBE_DRAW_STRING(400, 0, buf, VBE_AQUA, VBE_BLACK);
        VBE_UPDATE_VRAM();
    }

    DRAW_STRING("PS2 keyboard initialized successfully", VBE_GREEN);
    DRAW_STRING("RTOS kernel started", VBE_GREEN);

    // CMD_QUEUE *cmd_queue = GET_CMD_QUEUE();
    for(;;) {
    }

    HLT;
}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}