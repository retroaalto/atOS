; SOURCE\KERNEL\32RTOSKRNL\MEMORY.inc - Master memory include file for the 32RTOSKRNL
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Master memory include file for the 32RTOSKRNL.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/21 - Antonako1
;         Created this file with initial memory regions for KRNL and 32RTOSKRNL.
;     2025/05/21 - Antonako1
;         Updated memory regions based on the provided memory table,
;         adding new definitions and adjusting existing ones.
; 
; REMARKS
;     See .\MEMORY.md for a description of the memory regions.
%ifndef MEMORY_INC
%define MEMORY_INC

; --- 16-bit / early 32-bit memory regions ---

; Real Mode Interrupt Vector Table (IVT)
IVT_ADDRESS         equ     0x00000000
IVT_SIZE            equ     0x00000400         ; 1 KiB
IVT_END             equ     (IVT_ADDRESS + IVT_SIZE - 1)

; BIOS Data Area (BDA)
BIOS_DATA_AREA_ADDRESS equ  0x00000400
BIOS_DATA_AREA_SIZE equ     0x00000100         ; 256 bytes
BIOS_DATA_AREA_END  equ     (BIOS_DATA_AREA_ADDRESS + BIOS_DATA_AREA_SIZE - 1)

; Reserved BIOS scratchpad area
BIOS_RESERVED_ADDRESS equ   0x00000500
BIOS_RESERVED_SIZE  equ     0x00000B00         ; ~2.5 KiB (0xFFF - 0x500 + 1)
BIOS_RESERVED_END   equ     (BIOS_RESERVED_ADDRESS + BIOS_RESERVED_SIZE - 1)

; Bootloader (1st and 2nd stage)
BOOTLOADER_ADDRESS  equ     0x00001000
BOOTLOADER_SIZE     equ     0x00001000         ; 4 KiB
BOOTLOADER_END      equ     (BOOTLOADER_ADDRESS + BOOTLOADER_SIZE - 1)

; 32-bit kernel entry point
KRNL_ADDRESS        equ     0x00002000
KRNL_SIZE           equ     0x00006000         ; 24 KiB
KRNL_END            equ     (KRNL_ADDRESS + KRNL_SIZE - 1)

; BIOS memory map (E820)
E820_ADDRESS        equ     0x00008000
E820_SIZE           equ     0x00001000         ; 4 KiB
E820_END            equ     (E820_ADDRESS + E820_SIZE - 1)

; VESA BIOS Extensions (VBE) Mode Info block
VBE_MODE_INFO_ADDRESS equ   0x00009000
VBE_MODE_INFO_SIZE  equ     0x00000101         ; 257 bytes (0x9100 - 0x9000 + 1)
VBE_MODE_INFO_END   equ     (VBE_MODE_INFO_ADDRESS + VBE_MODE_INFO_SIZE - 1)

; VBE Optional controller block
VBE_CONTROLLER_INFO_ADDRESS equ 0x00009100
VBE_CONTROLLER_INFO_SIZE equ    0x00000F00     ; ~3.75 KiB (0x9FFF - 0x9100 + 1)
VBE_CONTROLLER_INFO_END equ     (VBE_CONTROLLER_INFO_ADDRESS + VBE_CONTROLLER_INFO_SIZE - 1)

; Legacy VGA framebuffer (e.g., Mode 13h)
LEGACY_VIDEO_MEMORY_ADDRESS equ 0x000A0000
LEGACY_VIDEO_MEMORY_SIZE equ    0x00020000     ; 128 KiB
LEGACY_VIDEO_MEMORY_END equ     (LEGACY_VIDEO_MEMORY_ADDRESS + LEGACY_VIDEO_MEMORY_SIZE - 1)

; BIOS ROM area + protected mode stack
BIOS_ROM_STACK_ADDRESS equ  0x000C0000
BIOS_ROM_STACK_SIZE equ     0x00040000         ; ~256 KiB
BIOS_ROM_STACK_END  equ     (BIOS_ROM_STACK_ADDRESS + BIOS_ROM_STACK_SIZE - 1)

; --- Protected mode / kernel memory ---

; Early kernel data, initial heap
DATA_ADDRESS        equ     0x00100000
DATA_SIZE           equ     0x00100000         ; 1 MiB
DATA_END            equ     (DATA_ADDRESS + DATA_SIZE - 1)

; 32-bit RTOS Kernel binary
RTOSKRNL_ADDRESS    equ     0x00200000
RTOSKRNL_SIZE       equ     0x00200000         ; 2 MiB
RTOSKRNL_END        equ     (RTOSKRNL_ADDRESS + RTOSKRNL_SIZE - 1)

; Kernel dynamic allocations
KERNEL_HEAP_ADDRESS equ     0x00400000
KERNEL_HEAP_SIZE    equ     0x00200000         ; 2 MiB
KERNEL_HEAP_END     equ     (KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE - 1)

; Temporary programs and data
PROGRAM_ADDRESS     equ     0x00600000
PROGRAM_SIZE        equ     0x00800000         ; 8 MiB
PROGRAM_END         equ     (PROGRAM_ADDRESS + PROGRAM_SIZE - 1)

; --- System structures and reserved memory ---

; Page directory + tables
PAGING_ADDRESS      equ     0x00E00000
PAGING_SIZE         equ     0x00100000         ; 1 MiB
PAGING_END          equ     (PAGING_ADDRESS + PAGING_SIZE - 1)

; VESA framebuffer (e.g., 1024x768x32)
FRAMEBUFFER_ADDRESS equ     0x00F00000
FRAMEBUFFER_SIZE    equ     0x00100000         ; 1 MiB
FRAMEBUFFER_END     equ     (FRAMEBUFFER_ADDRESS + FRAMEBUFFER_SIZE - 1)

; ACPI (RSDP/XSDT), local APIC structures
ACPI_ADDRESS        equ     0x01000000
ACPI_SIZE           equ     0x00100000         ; 1 MiB
ACPI_END            equ     (ACPI_ADDRESS + ACPI_SIZE - 1)

; Reserved for MMIO and future expansion
RESERVED_MMIO_ADDRESS equ   0x01100000
RESERVED_MMIO_SIZE  equ     0x06400000         ; 100 MiB
RESERVED_MMIO_END   equ     (RESERVED_MMIO_ADDRESS + RESERVED_MMIO_SIZE - 1)

; --- High memory user space ---

; Space for user applications, memory mappings, future use
USER_SPACE_ADDRESS  equ     0x07500000
USER_SPACE_END      equ     0x1FFFFFFF
USER_SPACE_SIZE     equ     (USER_SPACE_END - USER_SPACE_ADDRESS + 1) ; ~395 MiB

%endif ; MEMORY_INC
