#include "./VBE.h"


BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOURS colours) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(mode->PhysBasePtr); // byte pointer
    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;

    if (bytes_per_pixel >= 3) { // 24 or 32bpp
        framebuffer[pos + 0] = colours.BLUE;
        framebuffer[pos + 1] = colours.GREEN;
        framebuffer[pos + 2] = colours.RED;
        if (bytes_per_pixel == 4)
            framebuffer[pos + 3] = colours.ALPHA;
    } else { // 16bpp
        U16 color16 = VBE_COLOUR(colours.RED, colours.GREEN, colours.BLUE);
        *((U16*)(framebuffer + pos)) = color16;
    }

    return TRUE;
}


BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(mode->PhysBasePtr); // byte pointer
    U32 pitch = mode->BytesPerScanLineLinear;
    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;

    if (pixel_info.X >= mode->XResolution || pixel_info.Y >= mode->YResolution) {
        print_string("VBE_DRAW_PIXEL: Coordinates out of bounds.\n");
        return FALSE;
    }

    U32 offset = (pixel_info.Y * pitch) + (pixel_info.X * bytes_per_pixel);
    U32* pixel_ptr = (U32*)(framebuffer + offset);

    *pixel_ptr = (pixel_info.Colour.RED << 16) | 
                 (pixel_info.Colour.GREEN << 8) | 
                 pixel_info.Colour.BLUE;

    return TRUE;
}

