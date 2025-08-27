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

U0 kernel_after_gdt(U0);
__attribute__((noreturn))
void kernel_entry_main(U0) {
    vbe_check();
    
    VBE_DRAW_ELLIPSE(600, 600, 200, 50, VBE_PURPLE2);
    VBE_DRAW_ELLIPSE(650, 710, 180, 50, VBE_AQUA); VBE_STOP_DRAWING();

    GDT_INIT();                      // Setup GDT
    // setup in asm, can still be called
    kernel_after_gdt();
}

U0 kernel_after_gdt(U0) {
    IDT_INIT();                       // Setup IDT
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    VBE_FLUSH_SCREEN();
    VBE_DRAW_TRIANGLE(100, 100, 150, 300, 125, 70, VBE_WHITE);
    VBE_STOP_DRAWING();
    __asm__ volatile ("sti");        // Enable interrupts
    ASM_VOLATILE("hlt");
    VBE_DRAW_TRIANGLE(100, 100, 150, 300, 125, 70, VBE_WHITE);
    VBE_DRAW_LINE(0,0,100,100,VBE_RED);
    VBE_DRAW_RECTANGLE(500, 500, 100, 50, VBE_BLUE);
    VBE_STOP_DRAWING();

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

    VBE_DRAW_LINE(200, 200, 500, 500, VBE_RED);
    VBE_DRAW_LINE(530, 22, 40, 12, VBE_GREEN);
    
    
    for (U32 i = 0; i < 10; i++) {
        U32 radius = 10 - i;
        U32 centerY = mode->YResolution / 2;
        U32 centerX = mode->XResolution / 2;
    
        VBE_DRAW_LINE(centerX - radius, centerY, centerX + radius, centerY, VBE_AQUA);
        VBE_DRAW_LINE(centerX, centerY - radius, centerX, centerY + radius, VBE_AQUA);
    
        VBE_DRAW_LINE(centerX - radius, centerY - radius, centerX + radius, centerY + radius, VBE_AQUA);
        VBE_DRAW_LINE(centerX - radius, centerY + radius, centerX + radius, centerY - radius, VBE_AQUA);

        VBE_DRAW_LINE(i * 10, 0, i * 10, mode->YResolution, VBE_AQUA);
    }
        
    VBE_DRAW_TRIANGLE(100, 100, 150, 100, 125, 50, VBE_RED);

    VBE_DRAW_CHARACTER(100, 100, 0, VBE_WHITE, VBE_VIOLET);
    VBE_STOP_DRAWING();

    for(;;) ASM_VOLATILE("hlt");
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
