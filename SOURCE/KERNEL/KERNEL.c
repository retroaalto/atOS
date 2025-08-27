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
    #warning todo: tss_init();
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
        VBE_DRAW_CHARACTER(10 + i * 8, 10, str[i % sizeof(str)], VBE_WHITE, VBE_BLACK);
        VBE_STOP_DRAWING();
    }
    const char str2[] = "This is a test of the VBE graphics functions.";
    for (U32 i = 0; i < sizeof(str2); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 20, str2[i % sizeof(str2)], VBE_WHITE, VBE_BLACK);
        VBE_STOP_DRAWING();
    }
    const char str3[] = "See KERNEL.c!";
    for (U32 i = 0; i < sizeof(str3); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 30, str3[i % sizeof(str3)], VBE_WHITE, VBE_BLACK);
        VBE_STOP_DRAWING();
    }

    U32 ball_diameter = 30;
    F32 ball_x = 800;
    F32 ball_y = 50;         // start higher up
    F32 velocity_y = 0;      // current vertical velocity
    F32 gravity = 0.8f;      // pull down
    F32 bounce = -0.7f;      // lose some energy when bouncing
    F32 floor_y = 200;       // ground position

    for (U32 i = 0; i < 600; i++) {
        // Clear previous frame
        VBE_DRAW_RECTANGLE_FILLED(ball_x-ball_diameter, 0, 400, floor_y, VBE_CORAL);

        // Draw the ball
        VBE_DRAW_ELLIPSE((U32)ball_x, (U32)ball_y, ball_diameter/2, ball_diameter/2, VBE_RED);

        // Physics update
        velocity_y += gravity;         // apply gravity
        ball_y += velocity_y;          // update position

        if (ball_y + ball_diameter/2 >= floor_y) {
            ball_y = floor_y - ball_diameter/2; // snap to floor
            velocity_y *= bounce;               // bounce up with reduced speed
        }

        // Horizontal drift
        ball_x += 1;
        if(i % 2 == 0) {
            VBE_STOP_DRAWING();
        }
    }

    // Read RTOSKRNL.BIN;1 from disk
    // U32 *RTOSKRNL_ADDRESS = READ_ISO9660("RTOSKRNL.BIN;1");
    // Jump to RTOSKRNL.BIN;1
    // __asm__ volatile ("jmp %0" : : "r"(RTOSKRNL_ADDRESS));

    for(;;) ASM_VOLATILE("hlt");
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
