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
; CALCULATE_KRNL_SEGMENT:
;     xor eax, eax ; Clear eax
;     ; eax = edx + ebx * ecx
;     mov eax, edx ; eax = edx
;     mul ebx ; eax = eax * ebx
;     ; eax = eax * ebx

;     add eax, edx ; eax = eax + edx
;     ; eax = edx + ebx * ecx

;     ; Check if eax is less than 0x0000
;     cmp eax, 0x0000 ; Compare eax with 0x0000
;     jz .no_memory ; If eax is 0, there is no free memory
;     ret ; Return eax
; .no_memory:
;     stc ; Set carry flag
;     ret
    

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
    pusha                       ; Save all registers

    ; mov ds, eax                 ; Set DS to 0 for real mode addressing
    lea si, DAP                 ; Load DAP address into SI
    mov es, bx ; es:bx = buffer
    mov bx, dx

    ; ds:si= 0x0000:DAP

    mov byte [si],   0x14     ; DAP size
    mov byte [si+1], 0          ; Reserved
    mov word [si+2], cx         ; Number of sectors to read
    mov word [si+4], es         ; Buffer offset
    mov word [si+6], bx         ; Buffer segment
    mov dword [si+8], eax       ; LBA low 32 bits
    mov dword [si+12], 0         ; LBA high 16 bits (upper part of 48-bit LBA)
    mov byte [retry_count], 5   ; Max retries

.try:
    mov eax, 0x1
    call PRINT_HEX
    call PRINT_LINEFEED

    mov dl, [drive_number]      ; Drive number (e.g., 0x80)
    mov ah, 0x42                ; Extended Read
    int 0x13                    ; BIOS call
    jc .error                   ; Jump if CF=1 (error)

    popa                        ; Restore registers
    xor eax, eax                ; Success = 0
    ret
.error:
    dec byte [retry_count]
    jnz .try                    ; Retry if retries remain

    popa
    mov eax, 0xFFFF             ; Failure
    ret

%endif ; BIOS_ISO