; SOURCE/KERNEL/16-BIT-BIOS/STR.asm - String manipulation functions for 16-bit mode
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     String manipulation functions for 16-bit mode
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Added strncmp and strncopy functions
; 
; REMARKS
;     None
%ifndef BIOS_STR
%define BIOS_STR

; ax strncmp(DS:SI src, ES:DI dest, CX max_len)
; Returns: AX = 1 if equal up to CX chars or until NUL, AX = 0 if not equal
strncmp:
    push si
    push di
    push cx
.cmp_loop:
    cmp cx, 0
    je .equal
    lodsb                   ; AL = [DS:SI++]
    scasb                   ; compare AL to [ES:DI++]
    jne .not_equal
    test al, al              ; NUL terminator?
    jz .equal
    dec cx
    jmp .cmp_loop
.not_equal:
    mov ax, 0
    jmp .done
.equal:
    mov ax, 1
.done:
    pop cx
    pop di
    pop si
    ret

; void strncpy(DS:SI src, ES:DI dest, CX max_len)
; Copies up to CX chars, always NUL-terminates if CX > 0
strncpy:
    push si
    push di
    push cx
.copy_loop:
    cmp cx, 0
    je .done
    lodsb
    stosb
    test al, al
    jz .pad_zero
    dec cx
    jmp .copy_loop
.pad_zero:
    ; pad rest with NULs
    dec cx
    js .done
    mov al, 0
    rep stosb
.done:
    pop cx
    pop di
    pop si
    ret

%endif ; BIOS_STR
