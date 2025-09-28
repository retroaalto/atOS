#include <STD/TYPEDEF.h>
#include <DRIVERS/VIDEO/VBE.h>

U32 _start(U0) {
    VBE_DRAW_ELLIPSE(200, 200, 100, 50, VBE_WHITE);
    VBE_DRAW_STRING(0, 0, "Hello, World!", VBE_WHITE, VBE_BLACK);
    VBE_UPDATE_VRAM();
    return 1;
}