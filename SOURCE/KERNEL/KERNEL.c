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
#include "./32RTOSKRNL/DRIVERS/DISK/ATAPI/ATAPI.h"
#include "./32RTOSKRNL/DRIVERS/DISK/ATA/ATA.h"
#include "../STD/ASM.h"
#include "./32RTOSKRNL/MEMORY/E820.h"
#include "./32RTOSKRNL/CPU/INTERRUPTS.h"

void test(void) {
}


__attribute__((noreturn))
void kernel_entry_main(U0) {
    GDT_INIT();
    IDT_INIT();
    // IRQ_INIT();
    SETUP_ISR_HANDLERS();
    __asm__ volatile ("sti");

    // if(!vesa_check()) {
        // goto HLT_NO_DEBUG;
    // }
    // if(!vbe_check()) {
        // goto HLT_NO_DEBUG;
    // }

    // if(!E820_INIT()) {
        // goto HLT_NO_DEBUG;
    // }
    // TODO: GDT/IDT


    VBE_MODE* mode = GET_VBE_MODE();

    VBE_DRAW_ELLIPSE(
        mode->XResolution / 2, 
        mode->YResolution / 2, 
        mode->XResolution / 2, 
        mode->YResolution / 2, 
        VBE_CRIMSON
    );
    VBE_DRAW_ELLIPSE(100, 100, 90, 90, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 80, 80, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 70, 70, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 60, 60, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 50, 50, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 40, 40, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 30, 30, VBE_WHITE);
    VBE_DRAW_ELLIPSE(100, 100, 20, 20, VBE_BLACK);
    VBE_DRAW_ELLIPSE(100, 100, 10, 10, VBE_RED);
    VBE_DRAW_LINE(500, 500, 100, 100, VBE_GREEN);
    VBE_DRAW_LINE(100, 100, 300, 200, VBE_RED);
    VBE_DRAW_RECTANGLE(50, 50, 100, 100, VBE_RED);

    // VBE_DRAW_RECTANGLE_FILLED(200, 200, 10, 10, VBE_GREEN);
    // VBE_DRAW_LINE(200, 200, 500, 500, VBE_RED);
    // VBE_DRAW_LINE(530, 22, 40, 12, VBE_GREEN);
    
    
    for (U32 i = 0; i < 10; i++) {
        U32 radius = 10 - i;
        U32 centerY = mode->YResolution / 2;
        U32 centerX = mode->XResolution / 2;
    
        VBE_DRAW_LINE(centerX - radius, centerY, centerX + radius, centerY, VBE_AQUA);
        VBE_DRAW_LINE(centerX, centerY - radius, centerX, centerY + radius, VBE_AQUA);
    
        VBE_DRAW_LINE(centerX - radius, centerY - radius, centerX + radius, centerY + radius, VBE_AQUA);
        VBE_DRAW_LINE(centerX - radius, centerY + radius, centerX + radius, centerY - radius, VBE_AQUA);
    }
        
    // VBE_DRAW_LINE_THICKNESS(0, 0, 700, 500, VBE_RED, 5);
    VBE_DRAW_TRIANGLE(100, 100, 150, 100, 125, 50, VBE_RED);
        
    VBE_STOP_DRAWING();

HLT_NO_DEBUG:
    while (1) {
        __asm__ volatile ("hlt");
    }
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
