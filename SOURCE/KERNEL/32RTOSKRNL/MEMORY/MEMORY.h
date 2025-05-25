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
// REMARKS:
//     See MEMORY.md for a detailed description of the memory regions.

#ifndef MEMORY_H
#define MEMORY_H

// --- 16-bit / early 32-bit memory regions ---

#define IVT_ADDRESS              0x00000000UL
#define IVT_SIZE                 0x00000400UL  // 1 KiB
#define IVT_END                  (IVT_ADDRESS + IVT_SIZE - 1)

#define BIOS_DATA_AREA_ADDRESS   0x00000400UL
#define BIOS_DATA_AREA_SIZE      0x00000100UL  // 256 bytes
#define BIOS_DATA_AREA_END       (BIOS_DATA_AREA_ADDRESS + BIOS_DATA_AREA_SIZE - 1)

#define BIOS_RESERVED_ADDRESS    0x00000500UL
#define BIOS_RESERVED_SIZE       0x00000B00UL  // ~2.5 KiB (0xFFF - 0x500 + 1)
#define BIOS_RESERVED_END        (BIOS_RESERVED_ADDRESS + BIOS_RESERVED_SIZE - 1)

#define BOOTLOADER_ADDRESS       0x00001000UL
#define BOOTLOADER_SIZE          0x00001000UL  // 4 KiB
#define BOOTLOADER_END           (BOOTLOADER_ADDRESS + BOOTLOADER_SIZE - 1)

#define KRNL_ADDRESS             0x00002000UL
#define KRNL_SIZE                0x00006000UL  // 24 KiB
#define KRNL_END                 (KRNL_ADDRESS + KRNL_SIZE - 1)

#define E820_ADDRESS             0x00008000UL
#define E820_SIZE                0x00001000UL  // 4 KiB
#define E820_END                 (E820_ADDRESS + E820_SIZE - 1)

#define VESA_MODE_INFO_ADDRESS   0x00009000UL
#define VESA_MODE_INFO_SIZE      0x00000101UL  // 257 bytes
#define VESA_MODE_INFO_END       (VESA_MODE_INFO_ADDRESS + VESA_MODE_INFO_SIZE - 1)

#define VBE_CONTROLLER_INFO_ADDRESS 0x00009100UL
#define VBE_CONTROLLER_INFO_SIZE    0x00000F00UL  // ~3.75 KiB
#define VBE_CONTROLLER_INFO_END     (VBE_CONTROLLER_INFO_ADDRESS + VBE_CONTROLLER_INFO_SIZE - 1)

#define LEGACY_VIDEO_MEMORY_ADDRESS 0x000A0000UL
#define LEGACY_VIDEO_MEMORY_SIZE    0x00020000UL  // 128 KiB
#define LEGACY_VIDEO_MEMORY_END     (LEGACY_VIDEO_MEMORY_ADDRESS + LEGACY_VIDEO_MEMORY_SIZE - 1)

#define BIOS_ROM_STACK_ADDRESS   0x000C0000UL
#define BIOS_ROM_STACK_SIZE      0x00040000UL  // ~256 KiB
#define BIOS_ROM_STACK_END       (BIOS_ROM_STACK_ADDRESS + BIOS_ROM_STACK_SIZE - 1)

// --- Protected mode / kernel memory ---

#define DATA_ADDRESS             0x00100000UL
#define DATA_SIZE                0x00100000UL  // 1 MiB
#define DATA_END                 (DATA_ADDRESS + DATA_SIZE - 1)

#define RTOSKRNL_ADDRESS         0x00200000UL
#define RTOSKRNL_SIZE            0x00200000UL  // 2 MiB
#define RTOSKRNL_END             (RTOSKRNL_ADDRESS + RTOSKRNL_SIZE - 1)

#define KERNEL_HEAP_ADDRESS      0x00400000UL
#define KERNEL_HEAP_SIZE         0x00200000UL  // 2 MiB
#define KERNEL_HEAP_END          (KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE - 1)

#define PROGRAM_ADDRESS          0x00600000UL
#define PROGRAM_SIZE             0x00800000UL  // 8 MiB
#define PROGRAM_END              (PROGRAM_ADDRESS + PROGRAM_SIZE - 1)

// --- System structures and reserved memory ---

#define PAGING_ADDRESS           0x00E00000UL
#define PAGING_SIZE              0x00100000UL  // 1 MiB
#define PAGING_END               (PAGING_ADDRESS + PAGING_SIZE - 1)

#define FRAMEBUFFER_ADDRESS      0x00F00000UL
#define FRAMEBUFFER_SIZE         0x00100000UL  // 1 MiB
#define FRAMEBUFFER_END          (FRAMEBUFFER_ADDRESS + FRAMEBUFFER_SIZE - 1)

#define ACPI_ADDRESS             0x01000000UL
#define ACPI_SIZE                0x00100000UL  // 1 MiB
#define ACPI_END                 (ACPI_ADDRESS + ACPI_SIZE - 1)

#define RESERVED_MMIO_ADDRESS    0x01100000UL
#define RESERVED_MMIO_SIZE       0x06400000UL  // 100 MiB
#define RESERVED_MMIO_END        (RESERVED_MMIO_ADDRESS + RESERVED_MMIO_SIZE - 1)

// --- High memory user space ---

#define USER_SPACE_ADDRESS       0x07500000UL
#define USER_SPACE_END           0x1FFFFFFFUL
#define USER_SPACE_SIZE          (USER_SPACE_END - USER_SPACE_ADDRESS + 1)  // ~395 MiB

#endif // MEMORY_H
