[BITS 16]
[ORG 0x10000]

kernel_main:

    mov ah, 0x0E
    mov al, 'A'
    int 0x10

    cli
halt:
    jmp halt
