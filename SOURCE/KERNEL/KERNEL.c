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
#include "./32RTOSKRNL/KERNEL.h"
#include "./32RTOSKRNL/DRIVERS/VIDEO/VBE.h"

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




#define E820_TABLE_PHYS   0x8000u


/* -------------------- Inspectors -------------------- */
// typedef struct __attribute__((packed)) _GDTR32 {
//     U16 limit;
//     U32 base;
// } GDTR32;

// static void show_gdt_info(void) {
//     GDTR32 gdtr;
//     __asm__ volatile ("sgdt %0" : "=m"(gdtr));
//     print_string("[GDT]\n");
//     print_label_hex("  GDTR.limit", (U32)gdtr.limit);
//     print_label_hex("  GDTR.base ", gdtr.base);
// }

typedef struct __attribute__((packed)) _E820_ENTRY {
    U32 BaseAddressLow;
    U32 BaseAddressHigh;
    U32 LengthLow;
    U32 LengthHigh;
    U32 Type;
} E820_ENTRY;

static void show_e820_sample(void) {
    print_string("[E820]\n");
    E820_ENTRY* e = (E820_ENTRY*)(E820_TABLE_PHYS);

    if (e->LengthLow==0 && e->LengthHigh==0 && e->BaseAddressLow==0 && e->BaseAddressHigh==0) {
        print_string("  No E820 entries at 0x00008000\n");
        return;
    }

    U32 entry_count = 0;
    while(e->LengthLow || e->LengthHigh || e->BaseAddressLow || e->BaseAddressHigh) {
        print_label_u32("  E820 Entry", entry_count++);
        print_label_hex("    Base Address", (U32)e->BaseAddressLow);
        print_label_hex("    Length", (U32)e->LengthLow);
        print_label_hex("    Type", e->Type);
        e++;
    }
}






__attribute__((noreturn))
void kernel_entry_main(U0) {
    VIDEO_MODE = VIDEO_MODE_VESA;
    static const char hello[] = "\nHello from atOS-RT!\n";
    print_string(hello);
    print_crlf();


    // print_label_hex("Cursor Position", CURSOR);
    if(!vesa_check()) {
        print_string("[VESA] Check failed.\n");
        goto HALT_KRNL_ENTRY;
    }
    if(!vbe_check()) {
        goto HALT_KRNL_ENTRY;
    }


HALT_KRNL_ENTRY:
    VIDEO_MODE = VIDEO_MODE_VBE; // Set VBE mode flag
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);

    U32* framebuffer = (U32*)(mode->PhysBasePtr); // framebuffer base address

    while (1) {
        for (U32 y = 0; y < mode->YResolution; y++) {
            for (U32 x = 0; x < mode->XResolution; x++) {
                U32 offset = y * mode->XResolution + x;
                framebuffer[offset] = VBE_COLOUR_GREEN;
            }
        }
    }

}

__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
