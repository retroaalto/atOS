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
    
    Access VESA struct at memory address VESA_LOAD_ADDRESS_PHYS.

    When compiling, include VESA.c
---*/
#ifndef VESA_H
#define VESA_H
#include "../../../../STD/ATOSMINDEF.h" // TYPE DEFINITIONS
#include "../../MEMORY/MEMORY.h"

#define RM2LA(seg, off)  (((U32)(seg) << 4) + (U32)(off))
#define FAR_PTR_TO_LINEAR(ptr)  RM2LA(((ptr) >> 16) & 0xFFFF, (ptr) & 0xFFFF)

/*+++
external definitions
---*/
extern volatile U32 CURSOR; // Defined in KERNEL.c
#define VIDEO_MODE_VESA 0x00000001 // VESA mode flag
#define VIDEO_MODE_VBE  0x00000002 // VBE mode flag
extern volatile U32 VIDEO_MODE; // Defined in KERNEL.c

// #define VIDEO_MEMORY ((volatile U16*)0x00F00000)
#define VIDEO_MEMORY ((volatile U16*)0xB8000) // Text mode video memory address
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

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
    U32 VBE_AF_VERSION;
    DWORD VideoModes;      // Current video mode (16-bit mode number)
    U8 VBE_IMPLEMENTATION[216];
    U8 OEM_SCRATCHPAD[256]; // OEM scratchpad for additional data 
} __attribute__((packed)) VESA_INFO;

#define VESA_LOAD_ADDRESS_SEGMENT 0x9000u
#define VESA_LOAD_ADDRESS_OFFSET 0x0000u
#define VESA_LOAD_ADDRESS_PHYS VESA_LOAD_ADDRESS_SEGMENT * 16 + VESA_LOAD_ADDRESS_OFFSET
#define VESA_CTRL_SIZE 512
#define VESA_TARGET_MODE 0x116 // 1024x768x16

static inline BOOL vesa_check(void) {
    #ifdef KERNEL_ENTRY
    print_string("[VESA]\n");
    #endif
    VESA_INFO* vesa = (VESA_INFO*)(VESA_LOAD_ADDRESS_PHYS);
    // Check signature
    if (vesa->Signature[0] != 'V' || vesa->Signature[1] != 'E' ||
        vesa->Signature[2] != 'S' || vesa->Signature[3] != 'A') {
        #ifdef KERNEL_ENTRY
        print_label_hex("No VESA Signature at", VESA_LOAD_ADDRESS_PHYS);
        #endif
        return FALSE;
    }
    
    // print_string_len_label("  Signature", vesa->Signature, 4);
    if (vesa->Version < 0x0200) {
        #ifdef KERNEL_ENTRY
        print_string("  VESA version is too low, expected at least 2.0\n");
        #endif
        return FALSE;
    }
    #ifdef KERNEL_ENTRY
    print_label_hex("  VESA Version", vesa->Version);
    print_label_hex("  OEM String Pointer", vesa->OEMStringPtr);
    print_label_hex("  Capabilities", vesa->Capabilities);
    print_label_hex("  Video Mode Pointer", vesa->VideoModePtr);
    print_label_u32("  Total Memory (64KB blocks)", vesa->TotalMemory);
    // OEM Vendor Name
    U32 oem_vendor_ptr = vesa->OemVendorNamePtr;
    U32 linear_vendor  = FAR_PTR_TO_LINEAR(oem_vendor_ptr);
    print_string_len_label("  Oem Vendor Name", (CONST CHAR*)linear_vendor, 32);
    
    // OEM Product Name
    U32 oem_product_ptr = vesa->OemProductNamePtr;
    U32 linear_product  = FAR_PTR_TO_LINEAR(oem_product_ptr);
    print_string_len_label("  Oem Product Name", (CONST CHAR*)linear_product, 32);
    
    // OEM Product Revision
    U32 oem_rev_ptr = vesa->OemProductRevPtr;
    U32 linear_rev  = FAR_PTR_TO_LINEAR(oem_rev_ptr);
    print_string_len_label("  Oem Product Revision", (CONST CHAR*)linear_rev, 32);
    #endif
    
    U16 far_off_ptr = (U16)(vesa->VideoModePtr & 0xFFFF);
    U16 far_seg_ptr = (U16)((vesa->VideoModePtr >> 16) & 0xFFFF);
    #ifdef KERNEL_ENTRY
    print_label_hex("  far_off_ptr", far_off_ptr);
    print_label_hex("  far_seg_ptr", far_seg_ptr);
    #endif
    U32 linear_addr = RM2LA(far_seg_ptr, far_off_ptr);
    #ifdef KERNEL_ENTRY
    print_label_hex("  Video Modes (raw far pointer)", vesa->VideoModePtr);
    print_label_hex("  Linear Address", linear_addr);
    #endif
    U16 *mode_list = (U16*)linear_addr;
    for (int i = 0; mode_list[i] != 0xFFFF; i++) {
        if(mode_list[i] == VESA_TARGET_MODE) {
            #ifdef KERNEL_ENTRY
            print_string("  Found target VESA mode 0x116 (1024x768x32bpp)\n");
            #endif
            return TRUE;
            break;
        }
    }
    RETURN FALSE;
}

#endif