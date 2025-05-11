; SOURCE/KERNEL/16-BIT-BIOS/DATA.inc - BIOS data segment
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Data for the BIOS segment.
;
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/10 - Antonako1
;         Initial version.
; 
; REMARKS
;     None
%ifndef BIOS_DATA_SEGMENT
%define BIOS_DATA_SEGMENT

; DAP - Disk Access Protocol
;     This protocol is used to access the disk in 16-bit mode.
;     It is used to read and write sectors from the disk.
;     The protocol is defined in the BIOS.
DAP:
    db 0x10     ; Size of the DAP structure
    db 0        ; Reserved
    dw 4        ; Number of sectors to read/write
    dw 0x1000   ; Segment to load
    dw 0x0000   ; Offset to load (0x1000:0000 = 0x100000)
    dq 1        ; Logical block address (LBA)


; ----------------------------------------
; GDT structure: 3 entries (8 bytes each)
; ----------------------------------------
gdt_start:
gdt_null:                      ; Null descriptor (must be all 0)
    dq 0x0000000000000000

gdt_code:                      ; Code segment descriptor
    dw 0xFFFF                  ; Limit (bits 0–15)
    dw 0x0000                  ; Base (bits 0–15)
    db 0x00                    ; Base (bits 16–23)
    db 10011010b               ; Access byte: Present, Ring 0, Executable, Readable
    db 11001111b               ; Flags and Limit (bits 16–19): 4KB granularity, 32-bit
    db 0x00                    ; Base (bits 24–31)

gdt_data:                      ; Data segment descriptor
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b               ; Access byte: Present, Ring 0, Data, Writable
    db 11001111b
    db 0x00

gdt_end:

; ----------------------------------------
; GDTR structure: limit (2 bytes), base (4 bytes)
; ----------------------------------------
GDTR:
    dw gdt_end - gdt_start - 1    ; Size (GDT limit)
    dd gdt_start                  ; Address of the GDT base


%endif ; BIOS_DATA_SEGMENT