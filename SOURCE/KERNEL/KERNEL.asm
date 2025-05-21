; SOURCE/KERNEL/KERNEL.asm - 32-bit kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     32-bit kernel entry point
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

[BITS 32]
[org 0x2000]

start:
    mov esi, msg
    mov edi, 0xB8000       ; VGA text buffer
    call print_string

HANG:
    cli
    hlt
    jmp HANG


print_string:
.next:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0F           ; Light gray on black
    mov [edi], ax
    add edi, 2
    jmp .next
.done:
    ret

msg db "Kernel is running!", 0
