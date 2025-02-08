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

[ORG 0x1000]

jmp short kernel_entry

kernel_entry:
    mov si, kernelEntryMessage1
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

kernelEntryMessage1 db "Welcome to the OS kernel!", 0