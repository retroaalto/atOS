[BITS 16]
[ORG 0x7C00]

mov ah, 0x0E ; BIOS teletype function
mov al, 'A' ; Character to print
int 0x10 ; Call BIOS interrupt

jmp $ ; Infinite loop

times 510-($-$$) db 0
dw 0xAA55