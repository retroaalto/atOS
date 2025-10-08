#include <STD/GRAPHICS.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <STD/MEM.h>

void FLUSH_VRAM(VOID) {
    SYSCALL(SYSCALL_VBE_UPDATE_VRAM, 0, 0, 0, 0, 0);
}

BOOLEAN DRAW_8x8_CHARACTER(U32 x, U32 y, U8 ch, VBE_COLOUR fg, VBE_COLOUR bg) {
    PU32 m_x = MAlloc(sizeof(U32));
    PU32 m_y = MAlloc(sizeof(U32));
    PU8 m_ch = MAlloc(sizeof(U8));
    PU32 m_fg = MAlloc(sizeof(VBE_COLOUR));
    PU32 m_bg = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_x, &x, sizeof(U32));
    MEMCPY(m_y, &y, sizeof(U32));
    MEMCPY(m_ch, &ch, sizeof(U8));
    MEMCPY(m_fg, &fg, sizeof(VBE_COLOUR));
    MEMCPY(m_bg, &bg, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_CHARACTER, (U32)m_x, (U32)m_y, (U32)m_ch, (U32)m_fg, (U32)m_bg);
    Free(m_x);
    Free(m_y);
    Free(m_ch);
    Free(m_fg);
    Free(m_bg);
}


BOOLEAN DRAW_8x8_STRING(U32 x, U32 y, U8 *str, VBE_COLOUR fg, VBE_COLOUR bg) {
    if (!str) return;
    PU32 m_x = MAlloc(sizeof(U32));
    PU32 m_y = MAlloc(sizeof(U32));
    PU8 m_str = MAlloc(STRLEN(str) + 1);
    PU32 m_fg = MAlloc(sizeof(VBE_COLOUR));
    PU32 m_bg = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_x, &x, sizeof(U32));
    MEMCPY(m_y, &y, sizeof(U32));
    MEMCPY(m_str, str, STRLEN(str) + 1);
    MEMCPY(m_fg, &fg, sizeof(VBE_COLOUR));
    MEMCPY(m_bg, &bg, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_STRING, (U32)m_x, (U32)m_y, (U32)m_str, (U32)m_fg, (U32)m_bg);
    Free(m_x);
    Free(m_y);
    Free(m_str);
    Free(m_fg);
    Free(m_bg);
}
void CLEAR_SCREEN_COLOUR(VBE_COLOUR colour) {
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_colour, &colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_CLEAR_SCREEN, (U32)m_colour, 0, 0, 0, 0);
    Free(m_colour);
}
BOOLEAN DRAW_PIXEL(VBE_PIXEL_INFO info) {
    VBE_PIXEL_INFO *m_info = MAlloc(sizeof(VBE_PIXEL_INFO));
    if (!m_info) return;
    MEMCPY(m_info, &info, sizeof(VBE_PIXEL_INFO));
    SYSCALL(SYSCALL_VBE_DRAW_PIXEL, (U32)m_info, 0, 0, 0, 0);
    Free(m_info);
}
BOOLEAN DRAW_FRAMEBUFFER(U32 pos, VBE_COLOUR colour) {
    PU32 m_pos = MAlloc(sizeof(U32));
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_pos, &pos, sizeof(U32));
    MEMCPY(m_colour, &colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_FRAMEBUFFER, (U32)m_pos, (U32)m_colour, 0, 0, 0);
    Free(m_pos);
    Free(m_colour);
}
BOOLEAN DRAW_ELLIPSE(U32 x, U32 y, U32 rx, U32 ry, VBE_COLOUR colour) {
    PU32 m_x = MAlloc(sizeof(U32));
    PU32 m_y = MAlloc(sizeof(U32));
    PU32 m_rx = MAlloc(sizeof(U32));
    PU32 m_ry = MAlloc(sizeof(U32));
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_x, &x, sizeof(U32));
    MEMCPY(m_y, &y, sizeof(U32));
    MEMCPY(m_rx, &rx, sizeof(U32));
    MEMCPY(m_ry, &ry, sizeof(U32));
    MEMCPY(m_colour, &colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_ELLIPSE, (U32)m_x, (U32)m_y, (U32)m_rx, (U32)m_ry, (U32)m_colour);
    Free(m_x);
    Free(m_y);
    Free(m_rx);
    Free(m_ry);
    Free(m_colour);
}
BOOLEAN DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_COLOUR colour) {
    PU32 m_x1 = MAlloc(sizeof(U32));
    PU32 m_y1 = MAlloc(sizeof(U32));
    PU32 m_x2 = MAlloc(sizeof(U32));
    PU32 m_y2 = MAlloc(sizeof(U32));
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMSET(m_x1, x1, sizeof(U32));
    MEMSET(m_y1, y1, sizeof(U32));
    MEMSET(m_x2, x2, sizeof(U32));
    MEMSET(m_y2, y2, sizeof(U32));
    MEMSET(m_colour, colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_LINE, (U32)m_x1, (U32)m_y1, (U32)m_x2, (U32)m_y2, (U32)m_colour);
    Free(m_x1);
    Free(m_y1);
    Free(m_x2);
    Free(m_y2);
    Free(m_colour);
}
BOOLEAN DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_COLOUR colour) {
    PU32 m_x = MAlloc(sizeof(U32));
    PU32 m_y = MAlloc(sizeof(U32));
    PU32 m_width = MAlloc(sizeof(U32));
    PU32 m_height = MAlloc(sizeof(U32));
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_x, &x, sizeof(U32));
    MEMCPY(m_y, &y, sizeof(U32));
    MEMCPY(m_width, &width, sizeof(U32));
    MEMCPY(m_height, &height, sizeof(U32));
    MEMCPY(m_colour, &colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_RECTANGLE, (U32)m_x, (U32)m_y, (U32)m_width, (U32)m_height, (U32)m_colour);
    Free(m_x);
    Free(m_y);
    Free(m_width);
    Free(m_height);
    Free(m_colour);
}
BOOLEAN DRAW_FILLED_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_COLOUR colour) {
    for (U32 i = 0; i < height; i++) {
        DRAW_LINE(x, y + i, x + width - 1, y + i, colour);
    }
}
BOOLEAN DRAW_TRIANGLE(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_COLOUR colour) {
    PU32 m_x1 = MAlloc(sizeof(U32));
    PU32 m_y1 = MAlloc(sizeof(U32));
    PU32 m_x2 = MAlloc(sizeof(U32));
    PU32 m_y2 = MAlloc(sizeof(U32));
    PU32 m_x3 = MAlloc(sizeof(U32));
    PU32 m_y3 = MAlloc(sizeof(U32));
    PU32 m_colour = MAlloc(sizeof(VBE_COLOUR));
    MEMCPY(m_x1, &x1, sizeof(U32));
    MEMCPY(m_y1, &y1, sizeof(U32));
    MEMCPY(m_x2, &x2, sizeof(U32));
    MEMCPY(m_y2, &y2, sizeof(U32));
    MEMCPY(m_x3, &x3, sizeof(U32));
    MEMCPY(m_y3, &y3, sizeof(U32));
    MEMCPY(m_colour, &colour, sizeof(VBE_COLOUR));
    SYSCALL(SYSCALL_VBE_DRAW_LINE, (U32)m_x1, (U32)m_y1, (U32)m_x2, (U32)m_y2, (U32)m_colour);
    SYSCALL(SYSCALL_VBE_DRAW_LINE, (U32)m_x2, (U32)m_y2, (U32)m_x3, (U32)m_y3, (U32)m_colour);
    SYSCALL(SYSCALL_VBE_DRAW_LINE, (U32)m_x3, (U32)m_y3, (U32)m_x1, (U32)m_y1, (U32)m_colour);
    Free(m_x1);
    Free(m_y1);
    Free(m_x2);
    Free(m_y2);
    Free(m_x3);
    Free(m_y3);
    Free(m_colour);
}
BOOLEAN DRAW_FILLED_TRIANGLE(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_COLOUR colour) {
    U32 minx = x1;
    if (x2 < minx) minx = x2;
    if (x3 < minx) minx = x3;

    U32 maxx = x1;
    if (x2 > maxx) maxx = x2;
    if (x3 > maxx) maxx = x3;

    U32 miny = y1;
    if (y2 < miny) miny = y2;
    if (y3 < miny) miny = y3;

    U32 maxy = y1;
    if (y2 > maxy) maxy = y2;
    if (y3 > maxy) maxy = y3;

    /* Quick reject if bbox completely off-screen */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (maxx < 0 || maxy < 0) return FALSE; /* defensive, though U32 can't be <0 */
#pragma GCC diagnostic pop
    if (minx >= SCREEN_WIDTH || miny >= SCREEN_HEIGHT) return FALSE;
    /* Clip bounding box to screen */
    if (minx >= SCREEN_WIDTH) return FALSE;
    if (miny >= SCREEN_HEIGHT) return FALSE;

    U32 bx0 = minx;
    U32 by0 = miny;
    U32 bx1 = maxx;
    U32 by1 = maxy;

    if (bx0 >= SCREEN_WIDTH) bx0 = SCREEN_WIDTH - 1;
    if (by0 >= SCREEN_HEIGHT) by0 = SCREEN_HEIGHT - 1;
    if (bx1 >= SCREEN_WIDTH) bx1 = SCREEN_WIDTH - 1;
    if (by1 >= SCREEN_HEIGHT) by1 = SCREEN_HEIGHT - 1;

    /* Use integer edge functions (64-bit to avoid overflow) */
    const I32 ax = (I32)x1;
    const I32 ay = (I32)y1;
    const I32 bx = (I32)x2;
    const I32 by = (I32)y2;
    const I32 cx = (I32)x3;
    const I32 cy = (I32)y3;

    VBE_PIXEL_INFO p;
    p.Colour = colour;
    BOOLEAN any = FALSE;

    for (U32 yy = bx0 /* dummy init */; yy <= bx1; ++yy) { /* we will overwrite loops below */
        break;
    }
    /* iterate y then x within clipped bbox */
    for (U32 yy = by0; yy <= by1; ++yy) {
        for (U32 xx = bx0; xx <= bx1; ++xx) {
            /* compute edge functions relative to point (xx, yy) */
            I32 px = (I32)xx;
            I32 py = (I32)yy;

            I32 e0 = (bx - ax) * (py - ay) - (by - ay) * (px - ax); /* edge AB */
            I32 e1 = (cx - bx) * (py - by) - (cy - by) * (px - bx); /* edge BC */
            I32 e2 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx); /* edge CA */

            /* point is inside if all edge functions have same sign (or zero) */
            if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
                p.X = xx;
                p.Y = yy;
                if (DRAW_PIXEL(p)) any = TRUE;
            }
        }
    }

    return any;
}