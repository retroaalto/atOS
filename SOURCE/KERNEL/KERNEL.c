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
#define KERNEL_ENTRY
#include "./32RTOSKRNL/KERNEL.h"
#include "./32RTOSKRNL/DRIVERS/INPUT/KEYBOARD.h"
#include "./32RTOSKRNL/DEBUG/KDPRINT.h"
#include "ASM.h"

U0 kernel_after_gdt(U0);
__attribute__((noreturn))
void kernel_entry_main(U0) {
    kdprint_init();
    kdprint("[atOS] Kernel entry initialized\n");
    vesa_check();
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
    kdprint("[atOS] IRQs initialized\n");
    KBD_INIT();                       // Setup PS/2 keyboard driver
    kdprint("[atOS] Keyboard driver ready\n");
    #warning todo: tss_init();
    // tss_init();
    VBE_FLUSH_SCREEN();
    VBE_DRAW_TRIANGLE(100, 100, 150, 300, 125, 70, VBE_WHITE);
    VBE_STOP_DRAWING();
    __asm__ volatile ("sti");        // Enable interrupts
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
    for (U32 i = 0; i < 10; i++) {
        U32 radius = 10 - i;
        U32 centerY = mode->YResolution / 2;
        U32 centerX = mode->XResolution / 2;
        // Draw from left to right
        VBE_DRAW_LINE(0, centerY - radius, mode->XResolution, centerY - radius, VBE_AQUA);
        VBE_DRAW_LINE(0, centerY + radius, mode->XResolution, centerY + radius, VBE_AQUA);

        VBE_DRAW_LINE(0, centerY - radius, mode->XResolution, centerY - radius, VBE_AQUA);
        VBE_DRAW_LINE(0, centerY + radius, mode->XResolution, centerY + radius, VBE_AQUA);

        VBE_DRAW_LINE(0, centerY - radius, mode->XResolution, centerY - radius, VBE_AQUA);
    }
        
    VBE_DRAW_TRIANGLE(100, 100, 150, 100, 125, 50, VBE_RED);
    VBE_DRAW_RECTANGLE_FILLED(0,0, mode->XResolution, 200, VBE_BLACK);
    VBE_DRAW_TRIANGLE_FILLED(100, 100, 150, 100, 125, 50, VBE_RED);
    VBE_STOP_DRAWING();

    const char ascii_set[] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    for (U32 i = 0; i < sizeof(ascii_set); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 10, ascii_set[i], VBE_WHITE, VBE_BLACK);
    }
    VBE_STOP_DRAWING();
    // Little drawing animation:
    VBE_DRAW_RECTANGLE_FILLED(0,0, mode->XResolution, 200, VBE_BLACK);
    VBE_STOP_DRAWING();
    // const char str[] = "Hello from atOS's kernel entry!";
    // for (U32 i = 0; i < sizeof(str); i++) {
    //     VBE_DRAW_CHARACTER(10 + i * 8, 10, str[i % sizeof(str)], VBE_WHITE, VBE_BLACK);
    //     VBE_STOP_DRAWING();
    // }
    // const char str2[] = "This is a test of the VBE graphics functions.";
    // for (U32 i = 0; i < sizeof(str2); i++) {
    //     VBE_DRAW_CHARACTER(10 + i * 8, 20, str2[i % sizeof(str2)], VBE_WHITE, VBE_BLACK);
    //     VBE_STOP_DRAWING();
    // }
    const char str3[] = "Type something!";
    for (U32 i = 0; i < sizeof(str3); i++) {
        VBE_DRAW_CHARACTER(10 + i * 8, 30, str3[i % sizeof(str3)], VBE_WHITE, VBE_BLACK);
        VBE_STOP_DRAWING();
    }

    // U32 ball_diameter = 30;
    // F32 ball_x = 800;
    // F32 ball_y = 50;         // start higher up
    // F32 velocity_y = 0;      // current vertical velocity
    // F32 gravity = 0.8f;      // pull down
    // F32 bounce = -0.7f;      // lose some energy when bouncing
    // F32 floor_y = 200;       // ground position
    //
    // for (U32 i = 0; i < 600; i++) {
    //     // Clear previous frame
    //     VBE_DRAW_RECTANGLE_FILLED(ball_x-ball_diameter, 0, 400, floor_y, VBE_CORAL);
    //
    //     // Draw the ball
    //     VBE_DRAW_ELLIPSE((U32)ball_x, (U32)ball_y, ball_diameter/2, ball_diameter/2, VBE_RED);
    //
    //     // Physics update
    //     velocity_y += gravity;         // apply gravity
    //     ball_y += velocity_y;          // update position
    //
    //     if (ball_y + ball_diameter/2 >= floor_y) {
    //         ball_y = floor_y - ball_diameter/2; // snap to floor
    //         velocity_y *= bounce;               // bounce up with reduced speed
    //     }
    //
    //     // Horizontal drift
    //     ball_x += 1;
    //
    //     VBE_STOP_DRAWING();
    // }

    U32 cursor_x = 10;
    U32 cursor_y = 360;

    for(;;) {
        KBD_POLL();

        while(KBD_HAS_KEY()) {
            CHAR key = KBD_READ_KEY();
            if(key == '\n') {
                kdprint_char('\n');
                cursor_x = 10;
                cursor_y += 12;
                if(cursor_y >= mode->YResolution - 16) {
                    cursor_y = 60;
                }
                continue;
            }

            if(key == '\b') {
                if(cursor_x > 10) {
                    cursor_x -= 8;
                }
                VBE_DRAW_CHARACTER(cursor_x, cursor_y, ' ', VBE_WHITE, VBE_BLACK);
                VBE_STOP_DRAWING();
                kdprint_char('\b');
                kdprint_char(' ');
                kdprint_char('\b');
                continue;
            }

            kdprint_char(key);
            VBE_DRAW_CHARACTER(cursor_x, cursor_y, key, VBE_WHITE, VBE_BLACK);
            cursor_x += 8;
            if(cursor_x >= mode->XResolution - 8) {
                cursor_x = 10;
                cursor_y += 12;
                if(cursor_y >= mode->YResolution - 16) {
                    cursor_y = 60;
                }
            }
            VBE_STOP_DRAWING();
        }

        io_wait();
    }
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
