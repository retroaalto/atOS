; BOOTLOADER\BOOTLOADER.asm - Bootloader for the OS
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Bootloader for the OS. This is the first code that runs when the computer starts
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/06 - Antonako1
;         FAT16 Boot Sector Header and welcome message
;     2025/02/09 - Antonako1
;         Removed FAT16 Boot sector for ISO9660 header
; 
; REMARKS
;     None


[ORG 0x7C00]

start:
    cli                      ; Clear interrupts
    xor ax, ax               ; Zero out registers
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00           ; Stack at end of bootloader

    mov si, bootMessage
    call print_string

    ; halt
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string

done:
    ret

bootMessage db "Booting atOS revised technology...", 0

times 510-($-$$) db 0
dw 0xAA55
