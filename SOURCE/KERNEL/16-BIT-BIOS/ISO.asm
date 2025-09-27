; SOURCE/KERNEL/16-BIT-BIOS/ISO.asm - ISO9660 filesystem functions
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains functions for handling ISO9660 filesystem.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Initial version.
;
;     2025/08/27 - Antonako1
;         Fixed a bug with kernel load address and DAP.
;
; REMARKS
;     None
[BITS 16]
%ifndef BIOS_ISO
%define BIOS_ISO

%include "SOURCE/KERNEL/16-BIT-BIOS/DATA.inc"
%include "SOURCE/KERNEL/16-BIT-BIOS/BIOS.inc"
%include "SOURCE/KERNEL/16-BIT-BIOS/IO.asm"

; eax READ_DISK(eax LBA, cx sectors, dx:bx buffer)
;
; Description:
;   Reads sectors from the disk into the buffer.
; The function will return 0 if the read was successful, or an error code if it failed.
;
; Parameters:
;   eax - LBA of the first sector to read
;   cx - number of sectors to read
;   bx - Buffer offset
;   dx - Buffer segment
;
; Return:
;   eax - 0 if the read was successful
;   eax - 0xffff on error
READ_DISK:
    pusha
    
    lea si, [DAP]
    mov byte  [si], DAP_SIZE
    mov byte  [si+1], 0
    mov word  [si+2], cx    ; sectors
    mov word  [si+4], bx    ; buffer offset
    mov word  [si+6], dx    ; buffer segment
    mov dword [si+8], eax   ; LBA low
    mov dword [si+12], 0    ; LBA high
    mov byte [retry_count], 5

.try:
    mov dl, [drive_number]
    mov ah, 0x42
    int 0x13
    jc .error ; Carry flag set = error
    
    popa
    xor eax, eax            ; success = 0
    ret
.error:
    dec byte [retry_count]
    jnz .try ; Retry if retry_count > 0

    popa
    mov eax, 0xFFFF
    ret







%endif ; BIOS_ISO