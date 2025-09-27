; SOURCE/KERNEL/16-BIT-BIOS/DATA.inc - BIOS data segment
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Contains IDT, DAP and GDT.
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

;---------------------------------------------------------------
; DAP - Disk Address Packet (16 bytes)
;---------------------------------------------------------------
; Offset | Size | Description
; 0      | 1    | Size of packet (0x10)
; 1      | 1    | Reserved (always 0)
; 2      | 2    | Number of sectors to read
; 4      | 2    | Buffer offset (BX)
; 6      | 2    | Buffer segment (DX)
; 8      | 4    | Starting LBA, low 32 bits
; 12     | 4    | Starting LBA, high 16 bits
DAP:
    db 0x10           ; Size of DAP
    db 0x00           ; Reserved
    dw 0x0000         ; Number of sectors (set at runtime)
    dw 0x0000         ; Buffer offset (BX)
    dw 0x0000         ; Buffer segment (DX)
    dd 0x00000000     ; LBA low 32 bits
    dd 0x0000         ; LBA high 16 bits
    ; dd 0x0000

DAP_SIZE equ $ - DAP

; ----------------------------------------
; String Constants
; ----------------------------------------
msg_greeting_1:     db "16-bit kernel loaded...", 0
msg_kernel_end:     db "16-bit kernel ended, going into 32-bit protected mode...", 0
msg_test_1:         db "test", 0
msg_mem_err1:       db "E820h.", 0
msg_mem_err2:       db "E820h. EAX != 0x534D4150", 0
msg_mem_err3:       db "E820h. ECX < 20", 0
msg_mem_err4:       db "E820h. ECX > MEM_BUF_LEN", 0
msg_mem_err5:       db "No free memory for KRNL.BIN", 0
msg_mem_err6:       db "", 0
msg_hng_1:          db "_HANG_", 0
msg_e820_done:      db "E820h done...", 0
msg_disk_err1:      db "Error reading from disk...", 0
msg_iso_err1:       db "ISO error. Invalid volume descriptor", 0
msg_iso_err2:       db "ISO error. Invalid extent length", 0
msg_iso_err3:       db "ISO error. Invalid extent location", 0
msg_iso_errg:       db "ISO error. General", 0
msg_krnl_not_found: db "KRNL.BIN not found", 0
msg_vesa_err1:      db "VESA/VBE function error", 0
msg_vesa_err2:      db "VESA/VBE not supported", 0

; ----------------------------------------
; Boot drive number (set by BIOS)
; ----------------------------------------
drive_number:       db 0
DRIVE_NUMBER_MEM    equ 0x8000
DRIVE_NUMBER_SEGMENT equ 0x800
DRIVE_NUMBER_OFFSET  equ 0x0000

%endif ; BIOS_DATA_SEGMENT
