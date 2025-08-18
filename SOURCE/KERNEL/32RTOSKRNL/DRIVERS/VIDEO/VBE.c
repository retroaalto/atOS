#include "./VBE.h"
#include "../../../../STD/MATH.h"


BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOUR colour) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(mode->PhysBasePtr); // byte pointer
    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;
    
    if (pos >= mode->XResolution * mode->YResolution * bytes_per_pixel) {
        return FALSE;
    }

    switch (bytes_per_pixel) {
        case 2: // RGB565
            *(U16*)(framebuffer + pos) = (U16)colour;
            break;
        case 3: // RGB888
            framebuffer[pos + 0] = (colour & 0xFF);        // Blue
            framebuffer[pos + 1] = (colour >> 8) & 0xFF;   // Green
            framebuffer[pos + 2] = (colour >> 16) & 0xFF;  // Red
            break;
        case 4: // RGBA8888
            *(U32*)(framebuffer + pos) = (U32)colour;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}


BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;

    if(pixel_info.X == I32_MAX || pixel_info.Y == I32_MAX ||
        (I32)pixel_info.X < 0 || (I32)pixel_info.Y < 0 ||
        (U32)pixel_info.X >= mode->XResolution || (U32)pixel_info.Y >= mode->YResolution) {
        return FALSE;
    }

    U32 pos = (pixel_info.Y * mode->BytesPerScanLineLinear) + (pixel_info.X * bytes_per_pixel);
    return VBE_DRAW_FRAMEBUFFER(pos, pixel_info.Colour);
}

BOOLEAN VBE_DRAW_ELLIPSE(U32 x0, U32 y0, U32 a, U32 b, VBE_PIXEL_COLOUR fill_colours) {
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

BOOLEAN VBE_DRAW_LINE(U32 x0, U32 y0, U32 x1, U32 y1, VBE_PIXEL_COLOUR colour) {
    I32 dx = abs((I32)x1 - (I32)x0);
    I32 dy = abs((I32)y1 - (I32)y0);
    I32 sx = (x0 < x1) ? 1 : -1;
    I32 sy = (y0 < y1) ? 1 : -1;
    I32 err = dx - dy;

    I32 x = (I32)x0;
    I32 y = (I32)y0;

    while (1) {
        if(!VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, colour))) return FALSE;
        if (x == (I32)x1 && y == (I32)y1) break;
        I32 e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    return TRUE;
}





BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours) {
    if (width == 0 || height == 0) {
        return TRUE;
    }

    U32 errcnt = 0;

    if(!VBE_DRAW_LINE(x, y, width, y, colours)) errcnt++;
    if(!VBE_DRAW_LINE(width, y, width, height, colours)) errcnt++;
    if(!VBE_DRAW_LINE(width, height, x, height, colours)) errcnt++;
    if(!VBE_DRAW_LINE(x, height, x, y, colours)) errcnt++;

    return errcnt == 0;
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