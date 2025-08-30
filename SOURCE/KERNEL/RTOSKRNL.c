#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif
#include <RTOSKRNL.h>
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

#define INC_ROW(row) (row += VBE_CHAR_HEIGHT + 2)
#define DRAW_STRING(text, color) VBE_DRAW_STRING(0, row, text, color, VBE_BLACK); INC_ROW(row); VBE_UPDATE_VRAM();


__attribute__((noreturn))
void rtos_kernel(U0) {
    U32 row = 0;
    CLI;
    // Recalled here, to fix address issues
    vesa_check();
    vbe_check();
    GDT_INIT();
    IDT_INIT();
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    PIT_INIT();
    if(!PS2_KEYBOARD_INIT()) {
        DRAW_STRING("Failed to initialize PS2 keyboard", VBE_RED);
        HLT;
    }
    if(!E820_INIT()) {
        DRAW_STRING("Failed to initialize E820. Not enough memory.", VBE_RED);
        HLT;
    }
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

    CMD_QUEUE *cmd_queue = GET_CMD_QUEUE();
    for(;;) {
    }

    HLT;
}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}