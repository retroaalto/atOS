; SOURCE/KERNEL/KERNEL.asm - 32-bit kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     32-bit kernel entry point.
;     This file will read the kernel into 0x00200000 and jump to it.
;     Before that, disk driver will be initialized for reading that binary.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Created this file
;     2025/05/21 - Antonako1
;         Second stage bootloader jump to this 32-bit kernel entry point
; 
; REMARKS
;     Memory map at 0x9000-0xFFFF
;     This kernel at 0x2000-0x9000
;     Stack at 0x90000-0xA0000
;
;     Legacy... KERNEL.c is used instead of this file.
[BITS 32]
[org 0x2000]

start:
    mov ax, 0x10
    mov es, ax

    mov esi, msg_1
    mov ah, 0x0A
    call print_string

    cli
    ; try master drive
    mov al, ATA_MASTER
    mov cx, ATA_BASE
    call DETECT_ATA_TYPE
    cmp al, TYPE_ATAPI
    je .ATAPI_FOUND
    
    ;try slave drive
    mov al, ATA_SLAVE
    mov cx, ATA_BASE
    call DETECT_ATA_TYPE
    cmp al, TYPE_ATAPI
    je .ATAPI_FOUND

    jmp ERROR_GENERAL

.ATAPI_FOUND:

    ; ah = MASTER or SLAVE
    mov byte [MASTER_SLAVE], ah


    mov esi, msg_2
    mov ah, 0x0A
    call print_string

    jmp HANG
HANG:
    cli
    hlt
    jmp HANG

ERROR_GENERAL:
    mov esi, msg_err
    mov ah, 0xEC
    call print_string
    jmp HANG

; -----------------------
; Utility functions, used only in this file
; -----------------------
print_string:
    push eax
    mov eax, [ROW]
    mov ebx, 2
    mul ebx
    ; result in EAX
    mov ebx, 0xB8000
    add ebx, eax
    mov edi, ebx

    mov eax, [ROW]
    add eax, 80
    mov [ROW], eax
    pop eax
.next:
    lodsb
    test al, al
    jz .done
    ;mov ah, 0x0A           ; Green on black
    mov [edi], ax
    add edi, 2
    jmp .next
.done:
    ret




; -----------------------
; Includes
; -----------------------
%include "SOURCE/KERNEL/32RTOSKRNL/KERNEL.inc"