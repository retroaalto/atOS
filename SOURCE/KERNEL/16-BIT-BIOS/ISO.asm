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


; eax CALCULATE_KRNL_SEGMENT(ebx amount of sectors, ecx sector size, edx buffer_value)
;
; Description:
;     This function calculates the offset for the kernel segment.
;
; Parameters:
;     ebx - amount of sectors
;     ecx - sector size
;     edx - value of the buffer segment
;
; Return:
;     eax - segment of the first free memory after the E820h memory map
;     cf - 0 if there is free memory, 1 if there is no free memory.
CALCULATE_KRNL_SEGMENT:
    xor eax, eax ; Clear eax
    ; eax = edx + ebx * ecx
    mov eax, edx ; eax = edx
    mul ebx ; eax = eax * ebx
    ; eax = eax * ebx

    add eax, edx ; eax = eax + edx
    ; eax = edx + ebx * ecx

    ; Check if eax is less than 0x0000
    cmp eax, 0x0000 ; Compare eax with 0x0000
    jz .no_memory ; If eax is 0, there is no free memory
    ret ; Return eax
.no_memory:
    stc ; Set carry flag
    ret
    

; eax READ_DISK(eax LBA, cx sectors, bx:dx buffer)
;
; Descrition:
;     Reads sectors from the disk into the buffer.
;     The function will return 0 if the read was successful, or an error code if it failed.
;
; Parameters:
;     eax - LBA of the first sector to read
;     cx - number of sectors to read
;     bx - pointer to the buffer to read the sectors into. Buffer offset
;     dx - pointer to the buffer to read the sectors into. Buffer segment
;
; Return:
;     eax - 0 if the read was successful, or an error code if it failed.
READ_DISK:
    pusha
    mov si, DAP
    mov byte [si], 0x10 ; Drive number
    mov byte [si+1], 0                  ; Reserved
    mov word [si+2], cx ; Number of sectors to read
    mov word [si+4], bx ; Pointer to the buffer to read the sectors into. Buffer offset
    mov word [si+6], dx ; Pointer to the buffer to read the sectors into. Buffer segment
    mov dword [si+8], eax ; LBA of the first sector to read

    mov dl, [drive_number] ; Drive number
    mov ah, 0x42 ; Read disk sectors
    int 0x13 ; Call BIOS interrupt
    jc .error ; If carry flag is set, there was an error
    popa
    xor eax, eax ; Clear eax
    ret
.error:
    popa
    mov eax, 0xFFFF ; Set ax to error code
    ret

%endif ; BIOS_ISO