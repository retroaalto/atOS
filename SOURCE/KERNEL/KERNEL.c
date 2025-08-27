/*+++
    Source/KERNEL/KERNEL.c - 32-bit Kernel Entry Point

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit kernel entry point for atOS.

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

void itoa(U32 value, U8 *buffer, U32 base) {
    U32 i = 0;
    if (value == 0) {
        buffer[i++] = '0';
    } else {
        while (value != 0) {
            U32 digit = value % base;
            buffer[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
            value /= base;
        }
    }
    buffer[i] = '\0';
    for (U32 j = 0; j < i / 2; j++) {
        U8 temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}


U0 kernel_after_gdt(U0);
__attribute__((noreturn))
void kernel_entry_main(U0) {
    vesa_check();
    vbe_check();
    GDT_INIT();                      // Setup GDT
    // setup in asm, can still be called
    kernel_after_gdt();
    for(;;) ASM_VOLATILE("cli; hlt");
}

U0 kernel_after_gdt(U0) {
    
    IDT_INIT();                       // Setup IDT
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    // todo: tss_init();
    // tss_init();
    VBE_FLUSH_SCREEN();
    __asm__ volatile ("sti");        // Enable interrupts

    VBE_MODE* mode = GET_VBE_MODE();
    U32 *video_buffer = (U32*)mode->PhysBasePtr;
    video_buffer[0] = 0x00FF0000;
    VBE_DRAW_ELLIPSE(600, 600, 90, 90, VBE_WHITE);
    VBE_DRAW_ELLIPSE(600, 600, 80, 80, VBE_BLACK);
    VBE_DRAW_ELLIPSE(600, 600, 70, 70, VBE_WHITE);
    VBE_DRAW_ELLIPSE(600, 600, 60, 60, VBE_BLACK);
    VBE_DRAW_ELLIPSE(600, 600, 50, 50, VBE_WHITE);
    VBE_DRAW_ELLIPSE(600, 600, 40, 40, VBE_BLACK);
    VBE_DRAW_ELLIPSE(600, 600, 30, 30, VBE_WHITE);
    VBE_DRAW_ELLIPSE(600, 600, 20, 20, VBE_BLACK);
    VBE_DRAW_ELLIPSE(600, 600, 10, 10, VBE_RED);

    U32 row = 0;
    U32 status;
    #define rowinc row += VBE_CHAR_HEIGHT + 2
    status = ATAPI_CHECK();
    if (status == ATAPI_FAILED) {
        VBE_DRAW_STRING(0, row, "ATAPI check failed", VBE_WHITE, VBE_BLACK);
        UPDATE_VRAM();
        rowinc;
    } else {
        VBE_DRAW_CHARACTER(0, row, status, VBE_WHITE, VBE_BLACK);
        UPDATE_VRAM();
        rowinc;

        U16 buffer[ATAPI_SECTORS(1)] = {0}; // 1 sector = 1024 bytes
        U32 read = 0;
        if ((read = READ_CDROM(status, 16, 1, buffer)) != ATAPI_FAILED) {
            U8 *bytes = (U8*)buffer;
            for (int i = 0; i < 1000; i++) {
                if(i % 10 == 0)
                    rowinc;
                VBE_DRAW_CHARACTER(i * 40, row, bytes[i], VBE_WHITE, VBE_BLACK);
            }
            UPDATE_VRAM();
        } else {
            VBE_DRAW_STRING(0, row, "CD-ROM read failed", VBE_WHITE, VBE_BLACK);
            UPDATE_VRAM();
            rowinc;
        }
    }

    ASM_VOLATILE("cli;hlt");
    if(!ISO9660_IMAGE_CHECK()) {
        VBE_DRAW_STRING(0, row, "Invalid ISO9660 image", VBE_WHITE, VBE_BLACK);
        UPDATE_VRAM();
        rowinc;
    } else {
        VBE_DRAW_STRING(0, row, "Valid ISO9660 image", VBE_WHITE, VBE_BLACK);
        UPDATE_VRAM();
        rowinc;
    }

    U8 *drive_letter = (U8*)0x00008000;
    U32 *RTOSKRNL_ADDRESS = (U32*)MEM_RTOSKRNL_BASE; // Main kernel address
    U32 RTOSKRNL_SIZE = MEM_RTOSKRNL_END - MEM_RTOSKRNL_BASE;
    
    // Read RTOSKRNL.BIN;1 from disk
    BOOLEAN res = ISO9660_READFILE_TO_MEMORY("RTOSKRNL.BIN;1", RTOSKRNL_ADDRESS, &RTOSKRNL_SIZE);
    if(!res) {
        VBE_DRAW_STRING(0, 0, "Failed to load RTOSKRNL.BIN;1", VBE_WHITE, VBE_BLACK);
        UPDATE_VRAM();
    } else {
        // Jump to RTOSKRNL.BIN;1
        // __asm__ volatile ("jmp %0" : : "r"(RTOSKRNL_ADDRESS));
    }

    ASM_VOLATILE("cli;hlt");

    const char ascii_set[] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    for (U32 i = 0; i < sizeof(ascii_set); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 10, ascii_set[i], VBE_WHITE, VBE_BLACK);
    }
    VBE_STOP_DRAWING();
    
    
    // Little drawing animation:
    VBE_DRAW_RECTANGLE_FILLED(0,0, mode->XResolution, 200, VBE_BLACK);
    VBE_STOP_DRAWING();
    const char str[] = "Hello from atOS's kernel entry!";
    for (U32 i = 0; i < sizeof(str); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 10, str[i % sizeof(str)], VBE_WHITE, VBE_SEE_THROUGH);
        VBE_STOP_DRAWING();
    }
    const char str2[] = "This is a test of the VBE graphics functions.";
    for (U32 i = 0; i < sizeof(str2); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 20, str2[i % sizeof(str2)], VBE_WHITE, VBE_SEE_THROUGH);
        VBE_STOP_DRAWING();
    }
    const char str3[] = "See KERNEL.c!";
    for (U32 i = 0; i < sizeof(str3); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 30, str3[i % sizeof(str3)], VBE_WHITE, VBE_SEE_THROUGH);
        VBE_STOP_DRAWING();
    }



    for(;;) ASM_VOLATILE("hlt");
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
