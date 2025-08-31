// MEMORY.h - Master memory include file for 32RTOSKRNL
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
//
// DESCRIPTION:
//     Master memory definitions for 32RTOSKRNL memory regions.
//
// AUTHORS:
//     Antonako1
//
// REVISION HISTORY:
//     2025/05/26 - Antonako1
//         Created this file.
//     2025/08/19 - Antonako1
//         Updated memory regions.
//     2025/08/27 - Antonako1
//         Optimized memory map for protected mode, reuse low memory, moved kernel to 0x20000.
//     2025/08/30 - Antonako1
//         Added VESA/VBE structures, reduced reserved MMIO to 16 MiB, updated user space base.
//
// REMARKS:
//     See MEMORY.md for a detailed description of the memory regions.

#ifndef MEMORY_H
#define MEMORY_H

// Low Memory / BIOS areas
#define MEM_LOW_RESERVED_BASE      0x00000000
#define MEM_LOW_RESERVED_END       0x00000FFF

#define MEM_E820_BASE              0x00006000
#define MEM_E820_END               0x00006384

// VESA / VBE Info Structures
#define MEM_VESA_BASE              0x00006400
#define MEM_VESA_END               0x00007000

#define MEM_VESA_VBE_E820_PAGE_BASE 0x00006000
#define MEM_VESA_VBE_E820_PAGE_END   0x00007000

// Kernel entry point
#define MEM_KRNL_ENTRY_BASE        0x00100000
#define MEM_KRNL_ENTRY_END         0x00112000

// Main RTOS Kernel
#define MEM_RTOSKRNL_BASE          0x00100000
#define MEM_RTOSKRNL_END           0x00550000

// RTOS Kernel HEAP
#define MEM_RTOSKRNL_HEAP_BASE     0x00550000
#define MEM_RTOSKRNL_HEAP_END      0x00F44000

// BIOS Reserved
#define MEM_BIOS_BASE              0x000E0000
#define MEM_BIOS_END               0x00100000

// Framebuffer
#define MEM_FRAMEBUFFER_BASE       0x00F44000
#define MEM_FRAMEBUFFER_END        0x013004F0

// Reserved MMIO / firmware
#define MEM_RESERVED_BASE          0x01344000
#define MEM_RESERVED_END           0x02344000  // 16 MiB

// User Space
#define MEM_USER_SPACE_BASE        0x02344000
#define MEM_USER_SPACE_END_MIN     0x03D09000

#define MAX_ALLOWED_MEMORY         0x1FEFE0000

#endif // MEMORY_H
