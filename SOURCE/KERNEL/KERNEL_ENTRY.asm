; KERNEL\KERNEL_ENTRY.asm - Kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Kernel entry point. This is where the kernel starts executing
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/09 - Antonako1
;         Initial version. Prints a welcome message.
; 
; REMARKS
;     None


[BITS 16]
[ORG 0x1000]

start:
    sti
    mov sp, 0x1000
    mov si, msg_greeting_1
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

;DATA
msg_greeting_1:  db "Greetings from kernel!", 0