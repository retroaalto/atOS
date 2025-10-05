#include <STD/TYPEDEF.h>
#include <STD/ASM.h>
#include <DRIVERS/VIDEO/VBE.h>

U32 _start(U0) {
    U32 x = 0;
    U32 y = 0;
    while(1) {
        x = (x + 1) % SCREEN_WIDTH;
        if(x == 0) {
            y = (y + 20) % SCREEN_HEIGHT;
            if(y == 0) {
                VBE_CLEAR_SCREEN(VBE_BLACK);
            }
        }
        VBE_DRAW_LINE(x, y, x+100, y, VBE_RED);
        VBE_DRAW_STRING(10, 10, "red lines from atOShell", VBE_BLACK, VBE_GREEN);
        VBE_UPDATE_VRAM();
        // HLT;
    }
    return 1;
}