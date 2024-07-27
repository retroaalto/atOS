BITS 16

; boot0 initialization
start:
    cli
    cli             ; Clear interrupts
    xor ax, ax      ; Zero out the AX register
    mov ds, ax      ; Set DS to 0
    mov es, ax      ; Set ES to 0
    mov ss, ax      ; Set SS to 0
    mov sp, 0x7C00  ; Set stack pointer to 0x7C00
    sti             ; Enable interrupts
    jmp short canonicalized ;set cs and ip
canonicalized:
    mov ax, 0x07C0  ; Set data segment to where BIOS loads us
    mov ds, ax
    mov es, ax

        mov si, msg     ; Load address of message
print_string:
    lodsb           ; Load byte at [si] into al and increment si
    cmp al, 0       ; Check if we reached the end of the string
    je done         ; If al == 0, jump to done
    mov ah, 0x0E    ; BIOS teletype function
    mov bh, 0x00    ; Page number
    mov bl, 0x07    ; Text attribute (white on black)
    int 0x10        ; BIOS video interrupt
    jmp print_string; Loop back to print next character
done:
hang:
    jmp hang

;boot1 initialization

msg db 'Hello, World!', 0 ; The message to be printed

; Pad the rest of the 512-byte sector with zeros
times 510-($-$$) db 0
dw 0xAA55                   ; Boot signature
