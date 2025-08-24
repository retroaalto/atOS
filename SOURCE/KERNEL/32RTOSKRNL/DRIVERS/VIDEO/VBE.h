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

    Access VBE struct at memory address VBE_MODE_LOAD_ADDRESS_PHYS,
        or use macro GET_VBE_MODE()

    OS runs in VBE 116h, 1024x768x32bpp.
     Although allowing 32-bit real colours,
     use the 5:6:5 colour format and its macros.

    When compiling include VBE.c and VESA.c
    
---*/
#ifndef VBE_H
#define VBE_H

#include "./VESA.h"

#define VBE_MODE_OFFSET VESA_LOAD_ADDRESS_PHYS + VESA_CTRL_SIZE
#define VBE_MODE_LOAD_ADDRESS_PHYS VBE_MODE_OFFSET
#define VBE_MODE_SIZE 256


#define SCREEN_BPP 32

/*
Framebuffer is located at 0x00F00000u-0x011FFFFFu
It has a size of ~3MB, which is enough for the resolution 1024*768*(32/8)

Video memory data is written into this location, and updated to the vbe framebuffer.
*/
#define FRAMEBUFFER_ADDRESS MEM_FRAMEBUFFER_BASE
#define FRAMEBUFFER_END MEM_FRAMEBUFFER_END
#define FRAMEBUFFER_SIZE (FRAMEBUFFER_END - FRAMEBUFFER_ADDRESS + 1)


// 5:6:5 color format
/*
Usage as follows:
Red min-max: 0-31
Green min-max: 0-63
Blue min-max: 0-31
*/
#define VBE_COLOUR(r, g, b) \
    (U16)((r << 11) | (g << 5) | b)

#define DECONSTRUCT_VBE_COLOUR(colour, r, g, b) \
    do { \
        r = (colour >> 11) & 0x1F; \
        g = (colour >> 5) & 0x3F; \
        b = colour & 0x1F; \
    } while (0)

// Reds
#define VBE_RED          VBE_COLOUR(31, 0, 0)
#define VBE_DARK_RED     VBE_COLOUR(15, 0, 0)
#define VBE_LIGHT_RED    VBE_COLOUR(31, 16, 16)

// Greens
#define VBE_GREEN        VBE_COLOUR(0, 63, 0)
#define VBE_DARK_GREEN   VBE_COLOUR(0, 31, 0)
#define VBE_LIGHT_GREEN  VBE_COLOUR(16, 63, 16)

// Blues
#define VBE_BLUE         VBE_COLOUR(0, 0, 31)
#define VBE_DARK_BLUE    VBE_COLOUR(0, 0, 15)
#define VBE_LIGHT_BLUE   VBE_COLOUR(16, 16, 31)

// Yellows
#define VBE_YELLOW       VBE_COLOUR(31, 63, 0)
#define VBE_DARK_YELLOW  VBE_COLOUR(15, 31, 0)
#define VBE_LIGHT_YELLOW VBE_COLOUR(31, 63, 16)

// Cyans
#define VBE_CYAN         VBE_COLOUR(0, 63, 31)
#define VBE_DARK_CYAN    VBE_COLOUR(0, 31, 15)
#define VBE_LIGHT_CYAN   VBE_COLOUR(16, 63, 31)

// Magentas
#define VBE_MAGENTA      VBE_COLOUR(31, 0, 31)
#define VBE_DARK_MAGENTA VBE_COLOUR(15, 0, 15)
#define VBE_LIGHT_MAGENTA VBE_COLOUR(31, 16, 31)

// Oranges
#define VBE_ORANGE       VBE_COLOUR(31, 31, 0)
#define VBE_DARK_ORANGE  VBE_COLOUR(15, 15, 0)
#define VBE_LIGHT_ORANGE VBE_COLOUR(31, 47, 16)

// Browns
#define VBE_BROWN        VBE_COLOUR(19, 31, 0)
#define VBE_DARK_BROWN   VBE_COLOUR(9, 15, 0)
#define VBE_LIGHT_BROWN  VBE_COLOUR(23, 47, 8)

// Grays
#define VBE_GRAY         VBE_COLOUR(16, 32, 16)
#define VBE_LIGHT_GRAY   VBE_COLOUR(24, 48, 24)
#define VBE_DARK_GRAY    VBE_COLOUR(8, 16, 8)

// Black & White
#define VBE_BLACK        VBE_COLOUR(0, 0, 0)
#define VBE_WHITE        VBE_COLOUR(31, 63, 31)

// Additional common colors
#define VBE_PINK         VBE_COLOUR(31, 24, 28)
#define VBE_PURPLE       VBE_COLOUR(19, 0, 31)
#define VBE_PURPLE2      VBE_COLOUR(26, 0, 31)
#define VBE_LIME         VBE_COLOUR(16, 63, 16)
#define VBE_TEAL         VBE_COLOUR(0, 31, 31)
#define VBE_NAVY         VBE_COLOUR(0, 0, 19)
#define VBE_OLIVE        VBE_COLOUR(19, 31, 0)
#define VBE_MAROON       VBE_COLOUR(16, 0, 0)
#define VBE_AQUA         VBE_COLOUR(0, 31, 31)
#define VBE_SILVER       VBE_COLOUR(24, 48, 24)
#define VBE_GOLD         VBE_COLOUR(31, 51, 0)
#define VBE_CORAL        VBE_COLOUR(31, 31, 16)
#define VBE_INDIGO       VBE_COLOUR(8, 0, 16)
#define VBE_VIOLET       VBE_COLOUR(23, 0, 31)
#define VBE_BEIGE        VBE_COLOUR(24, 48, 16)
#define VBE_TAN          VBE_COLOUR(21, 40, 8)
#define VBE_KHAKI        VBE_COLOUR(24, 51, 8)
#define VBE_LAVENDER     VBE_COLOUR(28, 24, 28)
#define VBE_SALMON       VBE_COLOUR(31, 24, 16)
#define VBE_CRIMSON      VBE_COLOUR(31, 0, 8)
#define VBE_CYAN2        VBE_COLOUR(0, 63, 28)


// VBE_COLOUR macros
typedef U16 VBE_PIXEL_COLOUR;

typedef struct {
    U32 X;
    U32 Y;
    VBE_PIXEL_COLOUR Colour;
} VBE_PIXEL_INFO;

/// @brief Creates a pixel information structure.
/// Usage: CREATE_VBE_PIXEL_INFO(x, y, colour)
///     CREATE_VBE_PIXEL_INFO(100, 100, VBE_COLOUR_RED)
#define CREATE_VBE_PIXEL_INFO(x, y, colour) \
    ((VBE_PIXEL_INFO){(U32)(x), (U32)(y), (VBE_PIXEL_COLOUR)(colour)})


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

#define GET_VBE_MODE() ((VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS))

/*+++
STATIC INLINE BOOL vbe_check(U0)

DESCRIPTION
    Checks the validity of the VBE mode.
    This function verifies the VBE mode information structure and ensures
    that the necessary fields are populated correctly.

    This file contains bare minimum VBE support for 32-bit color modes, use 16-bit colours instead.

    Since this file is included by the much smaller KERNEL.c, it is important to ensure that
    only the bare-bones functionality is implemented here.
    The rest should be implemented in VIDEODRIVER.c and VIDEODRIVER.h.

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
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    // Check if the mode is valid
    if (mode->ModeAttributes == 0) {
        return FALSE;
    }
    // Check if the physical base pointer is valid
    if (mode->PhysBasePtr == 0) {
        return FALSE;
    }
    // Check if the mode is compatible with the screen size
    if (mode->XResolution < SCREEN_WIDTH || mode->YResolution < SCREEN_HEIGHT) {
        return FALSE;
    }
    return TRUE;
}


// memcpy copied from STD/MEM.h to avoid "bloating" the size of KRNL
U0 ___memcpy(void* dest, const void* src, U32 n);

// Allows for 255 characters
#define VBE_MAX_CHARS sizeof(CHAR) * 255
#define VBE_CHAR_HEIGHT 8
extern U8 VBE_LETTERS[VBE_MAX_CHARS][VBE_CHAR_HEIGHT];

BOOLEAN VBE_DRAW_CHARACTER(U32 x, U32 y, CHAR c, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg);

/*+++
U0 UPDATE_VRAM(U0);

DESCRIPTION
    Updates the VRAM by copying the contents of the framebuffer to the physical
    address specified by the VBE mode.

RETURN
    None.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/19 - Antonako1
        Initial version.

REMARKS
    This function is called to refresh the screen contents.
---*/
U0 UPDATE_VRAM(U0);


/*+++
U0 VBE_STOP_DRAWING(U0);

DESCRIPTION
    Calls once drawing is complete to finalize any changes and update the VRAM.

PARAMETERS
    U0

RETURN
    None.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/19 - Antonako1
        Initial version.

REMARKS
    This function is called to refresh the screen contents.
---*/
U0 VBE_STOP_DRAWING(U0);


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
BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOUR colours)

DESCRIPTION
    Draws a pixel on the framebuffer.

PARAMETERS
    U32    pos
        The position (offset) in the framebuffer.
    VBE_PIXEL_COLOUR colours
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
BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOUR colours);


/*+++
BOOLEAN VBE_DRAW_ELLIPSE(U32 x0, U32 y0, U32 a, U32 b, VBE_PIXEL_COLOUR colour)

DESCRIPTION
    Draws an ellipse on the framebuffer.

PARAMETERS
    U32    x0
        The x-coordinate of the center of the ellipse.
    U32    y0
        The y-coordinate of the center of the ellipse.
    U32    a
        Horizontal semi-axis length.
    U32    b
        The vertical semi-axis length.
    VBE_PIXEL_COLOUR colours
        The color information for the ellipse.

        RETURN
    TRUE if successful, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws an ellipse in the framebuffer.
---*/
BOOLEAN VBE_DRAW_ELLIPSE(
    U32 x0, 
    U32 y0, 
    U32 a, 
    U32 b, 
    VBE_PIXEL_COLOUR colour
);

/*+++
BOOLEAN VBE_DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_PIXEL_COLOUR colours)

DESCRIPTION
    Draws a line on the framebuffer.

PARAMETERS
    U32    x1
        The x-coordinate of the start point.
    U32    y1
        The y-coordinate of the start point.
    U32    x2
        The x-coordinate of the end point.
    U32    y2
        The y-coordinate of the end point.
    VBE_PIXEL_COLOUR colours
        The color information for the line.

RETURN
    TRUE if successful, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws a line in the framebuffer.
---*/
BOOLEAN VBE_DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_PIXEL_COLOUR colours);


/*+++
BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours)

DESCRIPTION
    Draws a rectangle on the framebuffer.

PARAMETERS
    U32    x
        The x-coordinate of the top-left corner.
    U32    y
        The y-coordinate of the top-left corner.
    U32    width
        The width of the rectangle.
    U32    height
        The height of the rectangle.
    VBE_PIXEL_COLOUR colours
        The color information for the rectangle.

RETURN
    TRUE if successful, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws a rectangle in the framebuffer.
---*/
BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours);

BOOLEAN VBE_DRAW_RECTANGLE_FILLED(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours);

/*+++
BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours)

DESCRIPTION
    Draws a triangle on the framebuffer.

PARAMETERS
    U32    x1
        The x-coordinate of the first vertex.
    U32    y1
        The y-coordinate of the first vertex.
    U32    x2
        The x-coordinate of the second vertex.
    U32    y2
        The y-coordinate of the second vertex.
    U32    x3
        The x-coordinate of the third vertex.
    U32    y3
        The y-coordinate of the third vertex.
    VBE_PIXEL_COLOUR colours
        The color information for the triangle.

RETURN
    TRUE if successful, FALSE otherwise.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version.

REMARKS
    This draws a triangle in the framebuffer.
---*/
BOOLEAN VBE_DRAW_TRIANGLE(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_PIXEL_COLOUR colours);

BOOLEAN VBE_DRAW_TRIANGLE_FILLED(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_PIXEL_COLOUR colours);

#define VBE_DRAW_LINE_THICKNESS(x1, y1, x2, y2, colours, thickness)              \
    do {                                                                         \
        I32 _dx = (I32)(x2) - (I32)(x1);                                         \
        I32 _dy = (I32)(y2) - (I32)(y1);                                         \
        float _len = sqrtf((float)(_dx * _dx + _dy * _dy));                      \
        if (_len == 0) {                                                         \
            VBE_DRAW_RECTANGLE((x1) - (thickness)/2, (y1) - (thickness)/2,       \
                               thickness, thickness, colours);                   \
        } else {                                                                 \
            float _px = -_dy / _len;  /* perpendicular unit x */                  \
            float _py =  _dx / _len;  /* perpendicular unit y */                  \
            float _ox = _px * ((float)(thickness) / 2.0f);                       \
            float _oy = _py * ((float)(thickness) / 2.0f);                       \
                                                                                  \
            /* Rectangle corners around the line */                               \
            I32 _x1a = (I32)((x1) + _ox);                                        \
            I32 _y1a = (I32)((y1) + _oy);                                        \
            I32 _x1b = (I32)((x1) - _ox);                                        \
            I32 _y1b = (I32)((y1) - _oy);                                        \
            I32 _x2a = (I32)((x2) + _ox);                                        \
            I32 _y2a = (I32)((y2) + _oy);                                        \
            I32 _x2b = (I32)((x2) - _ox);                                        \
            I32 _y2b = (I32)((y2) - _oy);                                        \
                                                                                  \
            /* Draw filled quad as two triangles */                               \
            VBE_DRAW_TRIANGLE(_x1a, _y1a, _x2a, _y2a, _x1b, _y1b, colours);      \
            VBE_DRAW_TRIANGLE(_x2a, _y2a, _x2b, _y2b, _x1b, _y1b, colours);      \
        }                                                                        \
    } while (0)

#endif