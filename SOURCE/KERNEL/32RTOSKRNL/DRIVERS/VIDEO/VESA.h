/*+++
    Source/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VESA.h - VESA Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    VESA (Video Electronics Standards Association) definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version. Contains VESA information structure and constants.

REMARKS
    None

DESCRIPTION
    VESA (Video Electronics Standards Association) definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/05/26 - Antonako1
        Created VESA information structure and constants.    

REMARKS
    This file should not be included alone, as VESA does not work without VBE.
    Use VBE.h to include VESA definitions in your application.

    Note: If building outside KERNEL, ensure that the external definitions are defined,
    if not, define them in your application's source file.
        See below for external definitions.
---*/
#ifndef VESA_H
#define VESA_H
#include "../../../../STD/ATOSMINDEF.h" // TYPE DEFINITIONS

/*+++
external definitions
---*/
extern volatile U32 CURSOR; // Defined in KERNEL.c
#define VIDEO_MODE_VESA 0x00000001 // VESA mode flag
#define VIDEO_MODE_VBE  0x00000002 // VBE mode flag
extern volatile U32 VIDEO_MODE; // Defined in KERNEL.c

// #define VIDEO_MEMORY ((volatile U16*)0x00F00000)
#define VIDEO_MEMORY ((volatile U16*)0xB8000) // Text mode video memory address
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* Foreground/Background colors */
#define COLOR_GREEN_ON_BLACK 0x0A

typedef struct {
    CHAR Signature[4];       // "VESA" signature (should be "VESA")
    U16 Version;        // VBE version (e.g., 0x0300 = 3.0)
    U32 OEMStringPtr;   // Pointer to an OEM string (e.g., manufacturer)
    U32 Capabilities;   // Flags: e.g., DAC width, graphics mode support
    U32 VideoModePtr;   // Pointer to a list of supported video modes (16-bit mode numbers)
    U16 TotalMemory;    // Number of 64KB blocks of video memory
    U16 Reserved[5];    // Reserved, must be ignored

    U32 OemSoftwareRev;      // OEM software revision
    U32 OemVendorNamePtr;    // Far pointer to vendor name string
    U32 OemProductNamePtr;   // Far pointer to product name string
    U32 OemProductRevPtr;    // Far pointer to product revision string
} __attribute__((packed)) VESA_INFO;

#define VESA_LOAD_ADDRESS_SEGMENT 0x9000u
#define VESA_LOAD_ADDRESS_OFFSET 0x0000u
#define VESA_LOAD_ADDRESS_PHYS VESA_LOAD_ADDRESS_SEGMENT * 16 + VESA_LOAD_ADDRESS_OFFSET
#define VESA_CTRL_SIZE 512
#define VESA_TARGET_MODE 0x107 // 1024x768x32bpp

static inline BOOL vesa_check(void) {
    print_string("[VESA]\n");
    VESA_INFO* vesa = (VESA_INFO*)(VESA_LOAD_ADDRESS_PHYS);
    // Check signature
    if (vesa->Signature[0] != 'V' || vesa->Signature[1] != 'E' ||
        vesa->Signature[2] != 'S' || vesa->Signature[3] != 'A') {
        print_label_hex("No VESA Signature at", VESA_LOAD_ADDRESS_PHYS);
        return FALSE;
    }
    
    // print_string_len_label("  Signature", vesa->Signature, 4);
    if (vesa->Version < 0x0200) {
        print_string("  VESA version is too low, expected at least 2.0\n");
        return FALSE;
    }
    print_label_hex("  VESA Version", vesa->Version);
    print_label_hex("  OEM String Pointer", vesa->OEMStringPtr);
    print_label_hex("  Capabilities", vesa->Capabilities);
    print_label_hex("  Video Mode Pointer", vesa->VideoModePtr);
    print_label_u32("  Total Memory (64KB blocks)", vesa->TotalMemory);

    // U32 *mode_list = (U32*)(vesa->VideoModePtr & 0xFFFFF);
    // for (int i = 0; mode_list[i] != 0xFFFF; i++) {
    //     print_label_hex("  Supported Video Mode", mode_list[i]);
    // }
    for( U32 i = 0; i < 5; i++) {
        print_label_hex("  Reserved", vesa->Reserved[i]);
    }
    
    return TRUE;
}

#endif