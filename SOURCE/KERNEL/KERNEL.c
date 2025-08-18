/*+++
    Source/KERNEL/KERNEL.c - 32-bit Kernel Entry Point

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit kernel entry point for atOS-RT.

AUTHORS
    Antonako1
    
REVISION HISTORY
    2025/05/26 - Antonako1
        Initial version. Switched from KERNEL.asm to KERNEL.c.
    2025/08/18 - Antonako1
        Displays VESA/VBE, E820, GDT and IDT information.
        Initializes video mode and clears the screen.

REMARKS
    None
---*/
#define KERNEL_ENTRY
#include "./32RTOSKRNL/KERNEL.h"
#include "./32RTOSKRNL/DRIVERS/VIDEO/VBE.h"
#include "./32RTOSKRNL/MEMORY/E820.h"
#include "./32RTOSKRNL/MEMORY/GDT_IDT.h"


volatile U32 CURSOR = 0; // Defined in VESA.h
volatile U32 VIDEO_MODE = 0; // Defined in VESA.h

void put_char(U8 ch) {
    if (ch == '\n') { CURSOR += (SCREEN_WIDTH - (CURSOR % SCREEN_WIDTH)); return; }
    if(CURSOR + 1 >= SCREEN_WIDTH * SCREEN_HEIGHT) {
        // Scroll the screen up by one line
        for (U32 i = 0; i < (SCREEN_WIDTH * (SCREEN_HEIGHT - 1)); i++) {
            VIDEO_MEMORY[i] = VIDEO_MEMORY[i + SCREEN_WIDTH];
        }
        // Clear the last line
        for (U32 i = SCREEN_WIDTH * (SCREEN_HEIGHT - 1); i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            VIDEO_MEMORY[i] = ((U16)COLOR_GREEN_ON_BLACK << 8) | ' ';
        }
        CURSOR -= SCREEN_WIDTH; // Adjust cursor position
    }
    VIDEO_MEMORY[CURSOR++] = ((U16)COLOR_GREEN_ON_BLACK << 8) | ch;
}

void clear_screen(U0) {
    for (U32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VIDEO_MEMORY[i] = ((U16)COLOR_GREEN_ON_BLACK << 8) | ' ';
    }
    CURSOR = 0;
}

void print_string(CONST CHAR* s) {
    for (U32 i = 0; s[i]; i++) {
        put_char((U8)s[i]);
    }
}

void print_string_len(CONST CHAR* s, U32 len) {
    for (U32 i = 0; i < len && s[i]; i++) {
        put_char((U8)s[i]);
    }
}

void print_string_len_label(CONST CHAR* label, CONST CHAR* s, U32 len) {
    print_string(label);
    print_string(": ");
    print_string_len(s, len);
    print_crlf();
}

void print_u32(U32 v) {
    U8 buf[10]; U32 i=0;
    if (v == 0) { put_char('0'); return; }
    while (v && i<10) { buf[i++] = '0' + (v % 10); v /= 10; }
    while (i) put_char(buf[--i]);
}

void print_label_u32(const char* label, U32 value) {
    print_string(label);
    print_string(": ");
    print_u32(value);
    print_crlf();
}

void print_hex32(U32 v) {
    for (int shift = 28; shift >= 0; shift -= 4) {
        U8 nib = (v >> shift) & 0xF;
        put_char(nib < 10 ? '0' + nib : 'A' + (nib - 10));
    }
}

void print_label_hex(const char* label, U32 value) {
    print_string(label);
    print_string(": 0x");
    print_hex32(value);
    print_crlf();
}



__attribute__((noreturn))
void kernel_entry_main(U0) {
    clear_screen();
    print_string("atOS-RT Kernel Entry Point\n");
    print_string("Kernel Version: 0.1.0\n");
    print_crlf();


    // print_label_hex("Cursor Position", CURSOR);
    if(!vesa_check()) {
        print_string("[VESA] Check failed.\n");
        goto HALT_KRNL_ENTRY;
    }
    if(!vbe_check()) {
        goto HALT_KRNL_ENTRY;
    }
    // TODO: E820
    // TODO: GDT/IDT


HALT_KRNL_ENTRY:
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);

    for(U32 y=0; y<mode->YResolution; y++) {
        for(U32 x=0; x<mode->XResolution; x++) {
            if(x % 2 == 0 && y % 2 == 0) {
                VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, VBE_YELLOW));
            } else if(x % 2 == 1 && y % 2 == 1) {
                VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, VBE_GREEN));
            } else {
                VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, VBE_BLACK));
            }
        }
    }
    VBE_DRAW_ELLIPSE(mode->XResolution / 2, mode->YResolution / 2, mode->XResolution / 2, mode->YResolution / 2, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 90, 90, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 80, 80, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 70, 70, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 60, 60, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 50, 50, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 40, 40, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 30, 30, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 20, 20, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 10, 10, VBE_RED);


    VBE_DRAW_LINE(0, 0, 700, 500, VBE_RED);
    // VBE_DRAW_LINE_THICKNESS(0, 0, 700, 500, VBE_RED, 5);
    // VBE_DRAW_RECTANGLE(50, 50, 100, 100, VBE_BLUE);
    // VBE_DRAW_TRIANGLE(100, 100, 150, 100, 125, 50, VBE_RED);

    print_string("Kernel entry point completed. Halting CPU...\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
