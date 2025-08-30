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
//
// REMARKS:
//     See MEMORY.md for a detailed description of the memory regions.

#ifndef MEMORY_H
#define MEMORY_H

// Low Memory / Reused BIOS areas
#define MEM_LOW_RESERVED_BASE      0x00000000
#define MEM_LOW_RESERVED_END       0x00000FFF

// Bootloader + Early Kernel Stub
#define MEM_BOOTLOADER_BASE        0x00001000
#define MEM_BOOTLOADER_END         0x00001FFF
#define MEM_KRNL_STUB_BASE         0x00002000
#define MEM_KRNL_STUB_END          0x00007FFF
#define MEM_E820_BASE              0x00008000
#define MEM_E820_END               0x00008FFF

#define MEM_DRIVE_LETTER_BASE      0x00008000
#define MEM_DRIVE_LETTER_END       0x00008001
#define EARLY_FREE_DATA_BASE       0x00008002
#define EARLY_FREE_DATA_END        0x000093FF
// Early Stack
#define MEM_STACK_BASE             0x00009400
#define MEM_STACK_END              0x0000A3FF

// Free / Temporary Buffers
#define MEM_MMIO_TEMP_BASE              0x0000A400
#define MEM_MMIO_TEMP_END               0x0000FFFF

#define MEM_FREE_1_BASE             0x00018000
#define MEM_FREE_1_END              0x00019FFF
// Kernel entry point
#define MEM_KRNL_BASE              0x00020000
#define MEM_KRNL_END               0x00024000

// RTOS kernel
#define MEM_RTOSKRNL_BASE          0x00028000
#define MEM_RTOSKRNL_END           0x00427FFF

// Kernel Heap
#define MEM_KERNEL_HEAP_BASE       0x00468000
#define MEM_KERNEL_HEAP_END        0x00667FFF

// Program / Temp
#define MEM_PROGRAM_TMP_BASE       0x00844000
#define MEM_PROGRAM_TMP_END        0x00DFFFFF

// Paging Structures
#define MEM_PAGING_BASE            0x00E44000
#define MEM_PAGING_END             0x00EFFFFF

// Framebuffer
#define MEM_FRAMEBUFFER_BASE       0x00F44000
#define MEM_FRAMEBUFFER_END        0x012004EF

// ACPI / APIC
#define MEM_ACPI_APIC_BASE         0x01245000
#define MEM_ACPI_APIC_END          0x012FFFFF

// Reserved
#define MEM_RESERVED_BASE          0x01344000
#define MEM_RESERVED_END           0x076FFFFF

// User Space
#define MEM_USER_SPACE_BASE        0x07744000
#define MEM_USER_SPACE_MIN         0x7A12000
// Ends in the E820 memory block
#endif // MEMORY_H
