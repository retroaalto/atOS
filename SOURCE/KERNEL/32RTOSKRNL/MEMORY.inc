; <PATH> - <ABOUT>
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Describe
; 
; AUTHORS
;     Name(s)
; 
; REVISION HISTORY
;     yyyy/mm/dd - Name(s)
;         Description
; 
; REMARKS
;     Additional remarks, if any.
%ifndef MEMORY_INC
%define MEMORY_INC

; --- 16-bit / early 32-bit memory regions ---

KRNL_ADDRESS        equ     0x00002000
KRNL_SIZE           equ     0x00006000         ; 24 KiB
KRNL_END            equ     (KRNL_ADDRESS + KRNL_SIZE - 1)

E820_ADDRESS        equ     0x00008000
E820_SIZE           equ     0x00001000         ; 4 KiB
E820_END            equ     (E820_ADDRESS + E820_SIZE - 1)

STACK_ADDRESS       equ     0x00090000
STACK_END           equ     0x00100000
STACK_SIZE          equ     (STACK_END - STACK_ADDRESS) ; 512 KiB

; --- Protected mode / kernel memory ---

DATA_ADDRESS        equ     0x00100000
DATA_END            equ     0x00200000
DATA_SIZE           equ     (DATA_END - DATA_ADDRESS)   ; 1 MiB

RTOSKRNL_ADDRESS    equ     0x00200000
RTOSKRNL_SIZE       equ     0x00200000                  ; 2 MiB
RTOSKRNL_END        equ     (RTOSKRNL_ADDRESS + RTOSKRNL_SIZE - 1)

KERNEL_HEAP_ADDRESS equ     0x00400000
KERNEL_HEAP_SIZE    equ     0x00200000                  ; 2 MiB
KERNEL_HEAP_END     equ     (KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE - 1)

PROGRAM_ADDRESS     equ     0x00600000
PROGRAM_END         equ     0x00E00000
PROGRAM_SIZE        equ     (PROGRAM_END - PROGRAM_ADDRESS) ; 8 MiB

; --- System structures and reserved memory ---

PAGING_ADDRESS      equ     0x00E00000
PAGING_SIZE         equ     0x00100000                  ; 1 MiB
PAGING_END          equ     (PAGING_ADDRESS + PAGING_SIZE - 1)

FRAMEBUFFER_ADDRESS equ     0x00F00000
FRAMEBUFFER_SIZE    equ     0x00100000                  ; 1 MiB
FRAMEBUFFER_END     equ     (FRAMEBUFFER_ADDRESS + FRAMEBUFFER_SIZE - 1)

ACPI_ADDRESS        equ     0x01000000
ACPI_SIZE           equ     0x00100000                  ; 1 MiB
ACPI_END            equ     (ACPI_ADDRESS + ACPI_SIZE - 1)

; --- High memory user space ---

USER_SPACE_ADDRESS  equ     0x01100000
USER_SPACE_END      equ     0x1FFFFFFF
USER_SPACE_SIZE     equ     (USER_SPACE_END - USER_SPACE_ADDRESS + 1) ; ~495 MiB

%endif ; MEMORY_INC
