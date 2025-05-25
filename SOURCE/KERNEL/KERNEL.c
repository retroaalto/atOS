/*+++
    Source/KERNEL/KERNEL.c - 32-bit Kernel Entry Point

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit kernel entry point for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/05/26 - Antonako1
        Initial version. Switched from KERNEL.asm to KERNEL.c.

REMARKS
    None
---*/
#include "../STD/ATOSMINDEF.h"

#define VIDEO_MEMORY ((VOLATILE U16*)0xB8000)
#define SCREEN_WIDTH 80

VOLATILE U32 ROW = 0;

U0 kernel_entry_main(U0) {
    print_string("Welcome to atOS-RT Kernel Entry!");
    
    while(1) {
        __asm__ ("hlt");
    }
}

U0 print_string(CONST UCHAR* str) {
    volatile unsigned short* video = VIDEO_MEMORY + ROW;
    ROW += SCREEN_WIDTH;

    while (*str) {
        U8 character = *str++;
        U8 attribute = 0x0A;
        *video++ = ((U16)attribute << 8) | character;
    }
}