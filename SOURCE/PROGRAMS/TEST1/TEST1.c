#include <STD/TYPEDEF.h>
#include <STD/ASM.h>
// #include <DRIVERS/VIDEO/VBE.h>
#include <PROGRAMS/TEST1/TEST1.h>

// Declare all functions above main to avoid CPU faults
U0 test_func_internal(U0);

U32 _start(U0) {
    // U32 x = 0;
    // U32 y = 0;
    // while(1) {
    //     x = (x + 1) % SCREEN_WIDTH;
    //     if(x == 0) {
    //         y = (y + 20) % SCREEN_HEIGHT;
    //         if(y == 0) {
    //             VBE_CLEAR_SCREEN(VBE_BLACK);
    //         }
    //     }
    //     VBE_DRAW_LINE(x, y, x+100, y, VBE_RED);
    //     VBE_DRAW_STRING(10, 10, "red lines from atOShell", VBE_BLACK, VBE_GREEN);
    //     test_func(); // from header
    //     test_func_internal();
    //     VBE_UPDATE_VRAM();
    // }
    return 1;
}

U0 test_func_internal(U0) {
    // VBE_DRAW_ELLIPSE(200, 200, 50, 30, VBE_BLUE);
    return;
}