/*+++
    Source/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VBE.h - VESA BIOS Extensions (VBE) Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit VESA BIOS Extensions (VBE) definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version. Contains VBE mode structure and constants.

REMARKS
    Dependant on VESA.h for VESA_INFO structure and constants.
    Use this header to include VBE mode definitions in your application.
---*/
#ifndef VBE_H
#define VBE_H

#include "./VESA.h"

/*+++

    VESA Pixel Access Macros
    These macros allow easy access to pixel data in VBE modes.
    Mode: VBE_MODE
    x: X coordinate of the pixel
    y: Y coordinate of the pixel
    Usage:
        VBE_PIXEL_PTR(mode, x, y) - Returns a pointer to the pixel at (x, y) in the specified mode.
    Example:
        U32 *pixel = VBE_PIXEL_PTR(mode, 100, 200);
        *pixel = 0x00FF00; // Set pixel to green

---*/
#define VBE_PIXEL_PTR(mode, x, y) \
    ((U32*)((U8*)(mode)->PhysBasePtr + ((y) * (mode)->XResolution + (x)) * 4))

#define VBE_COLOUR(r, g, b) \
    ((U32)((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF)))

#define VBE_COORDS(x, y) \
    ((U32)((((y) & 0xFFFF) << 16) | ((x) & 0xFFFF)))

#define VBE_COLOUR_COORDS(x, y, r, g, b) \
    ((U32)((((y) & 0xFFFF) << 16) | ((x) & 0xFFFF) | \
    (((r) & 0xFF) << 24) | (((g) & 0xFF) << 16) | ((b) & 0xFF)))
    
#define VBE_SET_PIXEL(mode, x, y, color) \
    *(VBE_PIXEL_PTR(mode, x, y)) = (color)

#define VBE_MODE_OFFSET VESA_LOAD_ADDRESS_PHYS + VESA_CTRL_SIZE
#define VBE_MODE_LOAD_ADDRESS_PHYS VBE_MODE_OFFSET
#define VBE_MODE_SIZE 256
#define VESA_TARGET_MODE 0x117 // 1024x768x32bpp

#define VBE_OFF_X_RESOLUTION       0x12
#define VBE_OFF_Y_RESOLUTION       0x14
#define VBE_OFF_BITS_PER_PIXEL     0x19
#define VBE_OFF_PHYS_BASE_PTR      0x28

// Colours
#define VBE_COLOUR_BLACK   0x000000
#define VBE_COLOUR_WHITE   0xFFFFFF
#define VBE_COLOUR_RED     0xFF0000
#define VBE_COLOUR_GREEN   0x00FF00
#define VBE_COLOUR_BLUE    0x0000FF
#define VBE_COLOUR_YELLOW  0xFFFF00
#define VBE_COLOUR_CYAN    0x00FFFF


typedef struct {
    U16 ModeAttributes;      // Mode attributes (bit flags)
    U8  WinAAttributes;      // Window A attributes
    U8  WinBAttributes;      // Window B attributes
    U16 WinGranularity;      // Window granularity in KB
    U16 WinSize;             // Window size in KB
    U16 WinASegment;         // Segment address of window A
    U16 WinBSegment;         // Segment address of window B
    U32 WinFuncPtr;          // Real mode pointer to window function
    U16 BytesPerScanLine;    // Bytes per scan line

    U16 XResolution;         // Horizontal resolution in pixels
    U16 YResolution;         // Vertical resolution in pixels
    U8  XCharSize;           // Character cell width
    U8  YCharSize;           // Character cell height
    U8  NumberOfPlanes;      // Number of memory planes
    U8  BitsPerPixel;        // Bits per pixel
    U8  NumberOfBanks;       // Number of banks
    U8  MemoryModel;         // Memory model (packed, direct, etc.)
    U8  BankSize;            // Bank size in KB
    U8  NumberOfImagePages;  // Number of images
    U8  Reserved1;           // Reserved

    U8  RedMaskSize;         // # of bits for red
    U8  RedFieldPosition;    // Position of red
    U8  GreenMaskSize;      // # of bits for green
    U8  GreenFieldPosition;  // Position of green
    U8  BlueMaskSize;        // # of bits for blue
    U8  BlueFieldPosition;   // Position of blue
    U8  RsvMaskSize;         // # of bits for reserved
    U8  RsvFieldPosition;    // Position of reserved

    U32 PhysBasePtr;         // Physical address of linear framebuffer
    U32 OffScreenMemOffset;  // Pointer to start of offscreen memory
    U16 OffScreenMemSize;    // Size of offscreen memory in KB
    U8  Reserved2[206];      // Padding / reserved
} __attribute__((packed)) VBE_MODE;

static inline BOOL vbe_check(void) {
    print_string("[VBE]\n");
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    VESA_INFO *vesa = (VESA_INFO*)(VESA_LOAD_ADDRESS_PHYS);
    
    // Check if the mode is valid
    if (mode->ModeAttributes == 0) {
        print_label_hex("  No valid VBE mode found at offset", VBE_MODE_LOAD_ADDRESS_PHYS);
        return FALSE;
    }
    
    print_label_hex("  Mode Attributes", mode->ModeAttributes);
    print_label_hex("  X Resolution", mode->XResolution);
    print_label_hex("  Y Resolution", mode->YResolution);
    print_label_hex("  Bits Per Pixel", mode->BitsPerPixel);
    print_label_hex("  Physical Base Pointer", mode->PhysBasePtr);
    print_label_u32("  Off-Screen Memory Size (KB)", mode->OffScreenMemSize);
    
    U16 *mode_list = (U16*)(((vesa->VideoModePtr & 0xFFFF0000) >> 12) +
                                  (vesa->VideoModePtr & 0xFFFF));
    for( U32 i = 0; i < 5; i++) {
        print_label_hex("  Video Mode", mode_list[i]);
    }


    // Check if the mode supports 32bpp
    if (mode->BitsPerPixel != 32) {
        print_string("  VBE mode does not support 32bpp.\n");
        return FALSE;
    }
    
    // Check if the physical base pointer is valid
    if (mode->PhysBasePtr == 0) {
        print_string("  Invalid Physical Base Pointer.\n");
        return FALSE;
    }
    
    print_string("  VBE mode check passed.\n");
    print_label_hex("  VBE Mode Offset, physical address", VBE_MODE_LOAD_ADDRESS_PHYS);
    print_label_hex("  VBE Mode Size", sizeof(VBE_MODE));
    
    // Check if the mode is compatible with the screen size
    if (mode->XResolution < SCREEN_WIDTH || mode->YResolution < SCREEN_HEIGHT) {
        print_string("[VBE] Resolution too low for screen.\n");
        return FALSE;
    }

    for(U32 x = 0; x < SCREEN_WIDTH; x++) {
        ((volatile U32*)(mode->PhysBasePtr))[x] = VBE_COLOUR_CYAN;
    }
    return TRUE;
}

U0 vbe_draw(U32 x, U32 y, U32 color);

#endif