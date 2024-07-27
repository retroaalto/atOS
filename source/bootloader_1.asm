BITS 16
ORG 0x7c00

%define endl 0x0d, 0x0a

start:
    jmp boot0


; ds:si points to a string
print:
    ; save registers
    push si
    push ax
print_loop:
    lodsb ; load char to al
    cmp al, 0
    je print_done
    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp print_loop
print_done:
    pop ax
    pop si
    ret   


boot0:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7c00
    sti
    mov si, msg_test
    call print
    jmp hang

hang:
    jmp hang

msg_test: db "Hello", endl, 0

times 510-($-$$) db 0
dw 0x0AA55