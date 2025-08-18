#include "./VBE.h"

U0 vbe_draw(U32 x, U32 y, U32 color) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_OFFSET);
    U32* pixel = VBE_PIXEL_PTR(mode, x, y);
    *pixel = color;
}
