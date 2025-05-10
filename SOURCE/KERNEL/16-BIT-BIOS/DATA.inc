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

%endif ; BIOS_DATA_SEGMENT