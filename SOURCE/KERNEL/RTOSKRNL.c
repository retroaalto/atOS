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

    if(!PAGEFRAME_INIT()) {
        DRAW_STRING("Failed to initialize page frame. Possibly not enough memory.", VBE_RED);
        HLT;
    }
    // INIT_PAGING();
    
    if(!PS2_KEYBOARD_INIT()) {
        DRAW_STRING("Failed to initialize PS2 keyboard", VBE_RED);
        HLT;
    }

    STI;

    U8 buf[100];
    U32 *pit_ticks = PIT_GET_TICKS_PTR();
    U32 i = 0;
    for(; i < 10; i++) {
        ITOA(*pit_ticks, buf, 10);
        VBE_DRAW_STRING(400, 0, buf, VBE_AQUA, VBE_BLACK);
        VBE_UPDATE_VRAM();
    }

    DRAW_STRING("PS2 keyboard initialized successfully", VBE_GREEN);
    DRAW_STRING("RTOS kernel started", VBE_GREEN);

    U32 strpos = 0;
    for (;;) {
        KEYPRESS kp = GET_CURRENT_KEY_PRESSED();
        U8 *keychar = KEYPRESS_TO_CHARS(&kp);
        if(kp.pressed == FALSE) continue; // Key released
        if(kp.keycode == KEY_BACKSPACE) {
            if(strpos > 0) {
                buf[--strpos] = '\0';
            }
            VBE_DRAW_RECTANGLE_FILLED(0, row, SCREEN_WIDTH, VBE_CHAR_HEIGHT + 2, VBE_BLACK);
            VBE_DRAW_STRING(0, row, buf, VBE_AQUA, VBE_BLACK);
            VBE_UPDATE_VRAM();
            continue;
        }
        if(!keychar || !keychar[0]) continue; // No key pressed or key released
        if(kp.keycode == KEY_UNKNOWN) continue; // Unknown key

        if(strpos >= sizeof(buf) - 1) {
            strpos = 0;
        }
        STRNCONCAT(buf, strpos++, keychar, sizeof(buf) - 1);


        VBE_DRAW_STRING(0, row, buf, VBE_AQUA, VBE_BLACK);
        VBE_UPDATE_VRAM();
    }


    HLT;
}

__attribute__((noreturn, section(".text")))
void _start(U0) {
    rtos_kernel();
    HLT;
}