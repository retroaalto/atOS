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
; REMARKS
;     None
[BITS 16]
%ifndef BIOS_ISO
%define BIOS_ISO

%include "SOURCE/KERNEL/16-BIT-BIOS/DATA.inc"
%include "SOURCE/KERNEL/16-BIT-BIOS/BIOS.inc"

; eax READ_DISK(eax LBA, cx sectors, bx:dx buffer)
;
; Descrition:
;     Reads sectors from the disk into the buffer.
;     The buffer must be 512-byte aligned (or 2048 for ISO9660).
;     The LBA must be 32-bit aligned.
;     The function will read the sectors from the disk and return the number of sectors read in eax.
;     The function will return 0 if the read was successful, or an error code if it failed.
;
; Parameters:
;     eax - LBA of the first sector to read
;     cx - number of sectors to read
;     bx - pointer to the buffer to read the sectors into.
;     dx - pointer to the buffer to read the sectors into.
;
; Return:
;     eax - number of sectors read
;     cf - 0 if the read was successful, 1 if it failed.
;     If cf = 1, then eax will contain the error code.
;     If cf = 0, then eax will contain the number of sectors read.
READ_DISK:
    lea si, DAP
    mov word [si+2], cx ; Number of sectors to read
    mov word [si+4], bx ; Pointer to the buffer to read the sectors into
    mov word [si+6], dx ; Pointer to the buffer to read the sectors into
    mov dword [si+8], eax ; LBA of the first sector to read

    mov ah, 0x42 ; Read disk sectors
    int 0x13 ; Call BIOS interrupt
    jc .error ; If carry flag is set, there was an error
    xor eax, eax ; Clear ax
    mov eax, [si+2] ; Number of sectors read
    clc ;clear carry flag
    ret
.error:
    mov ax, 0xFFFF ; Set ax to error code
    stc ; Set carry flag
    ret

%endif ; BIOS_ISO