[BITS 16]
[ORG 0x10000]

kernel_main:
    ; Set up a simple video mode and print a character

    mov ah, 0x0E                ; BIOS teletype output function
    mov al, 'A'                 ; Character to print
    int 0x10                    ; Call BIOS interrupt to print character

    ; Halt the system (infinite loop)
    cli                         ; Clear interrupts
halt:
    hlt                         ; Halt CPU
    jmp halt                    ; Infinite loop
