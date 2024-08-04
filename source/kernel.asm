[BITS 16]
ORG 0x0
start:
    sti
    mov si, msg_greeting
    call print
    jmp .hang
.hang:
    cli
    hlt
print:
    ; save registers
    push si
    push ax
print_loop:
    lodsb ; load char to al
    cmp al, 0
    je .print_done
    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp print_loop
.print_done:
    pop ax
    pop si
    ret   

msg_greeting:  db "Greetings from kernel!", 0