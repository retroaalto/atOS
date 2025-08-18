#include "./VBE.h"
#include "../../../../STD/MATH.h"


BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOUR colours) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(mode->PhysBasePtr); // byte pointer
    U16 color16 = colours;
    *((U16*)(framebuffer + pos)) = color16;
    return TRUE;
}


BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(mode->PhysBasePtr); // byte pointer
    U32 pitch = mode->BytesPerScanLineLinear;
    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;

    if (pixel_info.X >= mode->XResolution || pixel_info.Y >= mode->YResolution) {
        return FALSE;
    }
    if(
        pixel_info.X < 0 || 
        pixel_info.Y < 0 || 
        pixel_info.X >= I32_MAX ||
        pixel_info.Y >= I32_MAX
    ) {
        return FALSE;
    }

    U32 offset = (pixel_info.Y * pitch) + (pixel_info.X * bytes_per_pixel);
    U32* pixel_ptr = (U32*)(framebuffer + offset);

    *pixel_ptr = pixel_info.Colour;

    return TRUE;
}

BOOLEAN VBE_DRAW_ELLIPSE(
    U32 x0, U32 y0, U32 a, U32 b, VBE_PIXEL_COLOUR fill_colours
) {
    I32 x = 0;
    I32 y = b;
    U32 a2 = a*a;
    U32 b2 = b*b;
    I32 dx = 0;
    I32 dy = 2*a2*y;
    I32 d1 = b2 - a2*b + a2/4;

    while(dx < dy) {
        for(I32 px = -x; px <= x; px++) {
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 + y, fill_colours));
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 - y, fill_colours));
        }
        x++;
        dx += 2*b2;
        if(d1 < 0) {
            d1 += b2 + dx;
        } else {
            y--;
            dy -= 2*a2;
            d1 += b2 + dx - dy;
        }
    }

    I32 d2 = b2*(x*x + x) + a2*(y-1)*(y-1) - a2*b2;
    while(y >= 0) {
        for(I32 px = -x; px <= x; px++) {
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 + y, fill_colours));
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 - y, fill_colours));
        }
        y--;
        dy -= 2*a2;
        if(d2 > 0) {
            d2 += a2 - dy;
        } else {
            x++;
            dx += 2*b2;
            d2 += a2 - dy + dx;
        }
    }

    return TRUE;
}

BOOLEAN VBE_DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_PIXEL_COLOUR colours) {
    I32 dx = (I32)x2 - (I32)x1;
    I32 dy = (I32)y2 - (I32)y1;
    I32 abs_dx = abs(dx);
    I32 abs_dy = abs(dy);
    I32 sign_dx = (dx > 0) ? 1 : -1;
    I32 sign_dy = (dy > 0) ? 1 : -1;

    if (abs_dx > abs_dy) {
        I32 d = abs_dx / 2;
        for (I32 x = x1, y = y1; x != x2; x += sign_dx) {
            d += abs_dy;
            if (d >= abs_dx) {
                d -= abs_dx;
                y += sign_dy;
            }
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, colours));
        }
    } else {
        I32 d = abs_dy / 2;
        for (I32 x = x1, y = y1; y != y2; y += sign_dy) {
            d += abs_dx;
            if (d >= abs_dy) {
                d -= abs_dy;
                x += sign_dx;
            }
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, colours));
        }
    }
    return TRUE;
}




BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours) {
    if (width == 0 || height == 0) {
        return TRUE;
    }

    VBE_DRAW_LINE(x, y, width, y, colours);
    // VBE_DRAW_LINE(width, y, width, height, colours);
    // VBE_DRAW_LINE(width, height, x, height, colours);
    // VBE_DRAW_LINE(x, height, x, y, colours);

    return TRUE;
}
BOOLEAN VBE_DRAW_TRIANGLE(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_PIXEL_COLOUR colours) {
    I32 _ = x1+y1-x2-y2+x3-y3+colours;
    _+=1;
    return TRUE;
}


/* Saved for later:
Bugs and draws out line patterns
BOOLEAN VBE_DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_PIXEL_COLOUR colours, U32 thickness) {
    I32 t_2 = (I32)thickness / 2;
    I32 A = (I32)y2 - (I32)y1;
    I32 B = (I32)x1 - (I32)x2;
    I32 C = (I32)x2 * (I32)y1 - (I32)x1 * (I32)y2;
    I32 A2 = A * A;
    I32 B2 = B * B;

    I32 xmin = (I32)x1 < (I32)x2 ? (I32)x1 : (I32)x2;
    I32 xmax = (I32)x1 > (I32)x2 ? (I32)x1 : (I32)x2;
    I32 ymin = (I32)y1 < (I32)y2 ? (I32)y1 : (I32)y2;
    I32 ymax = (I32)y1 > (I32)y2 ? (I32)y1 : (I32)y2;

    xmin -= t_2; xmax += t_2;
    ymin -= t_2; ymax += t_2;

    I32 threshold2 = pow2(t_2) * (A2 + B2);
    VBE_MODE* mode = GET_VBE_MODE();
    for(I32 x = xmin; x <= xmax; x++) {
        for(I32 y = ymin; y <= ymax; y++) {
            if(x < 0 || y < 0 || x >= mode->XResolution || y >= mode->YResolution) continue;
            I32 d = A * x + B * y + C;
            if (d*d <= threshold2) {
                VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, colours));
            }
        }
    }
    return TRUE;
}
*/