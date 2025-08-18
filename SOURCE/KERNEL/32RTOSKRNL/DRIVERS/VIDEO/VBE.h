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

    Access VBE struct at memory address VBE_MODE_LOAD_ADDRESS_PHYS.
---*/
#ifndef VBE_H
#define VBE_H

#include "./VESA.h"

#define VBE_MODE_OFFSET VESA_LOAD_ADDRESS_PHYS + VESA_CTRL_SIZE
#define VBE_MODE_LOAD_ADDRESS_PHYS VBE_MODE_OFFSET
#define VBE_MODE_SIZE 256

// 5:6:5 color format
#define VBE_COLOUR(r, g, b) \
    ((U16)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define VBE_COLOUR_RED   VBE_COLOUR(255, 0, 0) // Red in 5:6:5 format
#define VBE_COLOUR_GREEN VBE_COLOUR(0, 255, 0) // Green in 5:6:5 format
#define VBE_COLOUR_BLUE  VBE_COLOUR(0, 0, 255) // Blue in 5:6:5 format

// 32-bit color format
#define VBE_COLOUR32(r, g, b, a) \
    ((U32)((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF) | (((a) & 0xFF) << 24)))

#define VBE_COLOUR_RED_32   VBE_COLOUR32(255, 0, 0) // Red in 32-bit format
#define VBE_COLOUR_GREEN_32 VBE_COLOUR32(0, 255, 0) // Green in 32-bit format
#define VBE_COLOUR_BLUE_32  VBE_COLOUR32(0, 0, 255) // Blue in 32-bit format

typedef struct {
    U8 RED;
    U8 GREEN;
    U8 BLUE;
    U8 ALPHA;
} __attribute__((packed)) VBE_PIXEL_COLOURS;

// Creates colour structure for 32-bit colouring. Use VBE_COLOUR32
#define CREATE_VBE_PIXEL_COLOURS32(colour) \
    ((VBE_PIXEL_COLOURS){(U8)((colour) >> 16), (U8)((colour) >> 8), (U8)(colour), (U8)((colour) >> 24)})

// Creates colour structure for 16-bit colouring. Use VBE_COLOUR
#define CREATE_VBE_PIXEL_COLOURS(colour) \
    ((VBE_PIXEL_COLOURS){(U8)((colour) >> 8), (U8)(colour), 0xFF})

typedef struct {
    U32 X;
    U32 Y;
    VBE_PIXEL_COLOURS Colour;
} VBE_PIXEL_INFO;

#define CREATE_VBE_PIXEL_INFO(x, y, r, g, b, a) \
    ((VBE_PIXEL_INFO){(U32)(x), (U32)(y), CREATE_VBE_PIXEL_COLOURS32(VBE_COLOUR32(r, g, b, a))})


/// @brief VESA BIOS Extensions (VBE) mode information
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

    /* remainder is optional for VESA modes in v1.0/1.1, needed for OEM modes */
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

    /* VBE 1.2+ */
    U8  RedMaskSize;         // # of bits for red
    U8  RedFieldPosition;    // Position of red
    U8  GreenMaskSize;      // # of bits for green
    U8  GreenFieldPosition;  // Position of green
    U8  BlueMaskSize;        // # of bits for blue
    U8  BlueFieldPosition;   // Position of blue
    U8  RsvMaskSize;         // # of bits for reserved
    U8  RsvFieldPosition;    // Position of reserved
    U8  DirectColorModeInfo; // Direct color mode info
    /* Bit 0: 0 = Direct Color, 1 = Pseudo Color */

    /* VBE 2.0+ */
    U32 PhysBasePtr;         // Physical address of linear framebuffer
    U32 OffScreenMemOffset;  // Pointer to start of offscreen memory
    U16 OffScreenMemSize;    // Size of offscreen memory in KB

    /* VBE 3.0+ */
    U16 BytesPerScanLineLinear; // Bytes per scan line in linear mode
    U8 NumOfImagesBanked;          // Number of images in banked mode
    U8 NumOfImagesLinear;          // Number of images in linear mode
    U8 LinearModesRedMaskSz;     // Red mask size in linear mode
    U8 LinearModesRedLsb; // Red LSB position in linear mode
    U8 LinearModesGreenMaskSz;   // Green mask size in linear mode
    U8 LinearModesGreenLsb;       // Green LSB position in linear mode
    U8 LinearModesBlueMaskSz;     // Blue mask size in linear mode
    U8 LinearModesBlueLsb;        // Blue LSB position in linear mode
    U8 LinearModesRsvMaskSz;      // Reserved mask size in linear mode
    U8 LinearModesRsvLsb;         // Reserved LSB position in linear mode
    U32 PhysBasePtr2;             // Second physical base pointer (if applicable)
} __attribute__((packed)) VBE_MODE;

/*+++
STATIC INLINE BOOL vbe_check(U0)

DESCRIPTION
    Checks the validity of the VBE mode.
    This function verifies the VBE mode information structure and ensures
    that the necessary fields are populated correctly.

RETURN
    TRUE if the VBE mode is valid, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS:
    This function is called during the initialization phase to ensure
    that the VBE mode is set up correctly before use.
---*/
STATIC INLINE BOOL vbe_check(U0) {
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

    U32 *framebuffer = (U32*)(mode->PhysBasePtr); // framebuffer base address
    // Fill the screen with cyan color as a test
    for(U32 y = 0; y < SCREEN_HEIGHT; y++) {
        for(U32 x = 0; x < SCREEN_WIDTH; x++) {
            framebuffer[y * SCREEN_WIDTH + x] = VBE_COLOUR_RED;
        }
    }
    return TRUE;
}

/*+++
BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info)

DESCRIPTION
    Draws a pixel on the framebuffer.

PARAMETERS
    VBE_PIXEL_INFO    pixel_info
        Information about the pixel to draw.

RETURN
    True if successful, False otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws a pixel in the frambuffer into the given coordinates inside pixel info
---*/
BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info);


/*+++
BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOURS colours)

DESCRIPTION
    Draws a pixel on the framebuffer.

PARAMETERS
    U32    pos
        The position (offset) in the framebuffer.
    VBE_PIXEL_COLOURS colours
        The color information for the pixel.

RETURN
    TRUE if successful, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws a pixel in the framebuffer, not at the current cursor position nor at coordinates.
---*/
BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOURS colours);

#endif