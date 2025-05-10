; SOURCE/KERNEL/16-BIT-BIOS/IO.asm - BIOS I/O functions
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains BIOS I/O functions for 16-bit mode.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Initial version. 
; 
; REMARKS
;     None

%ifndef BIOS_IO
%define BIOS_IO

; void PRINTN(DS:SI str, cx len);
; 
; DESCRIPTION
;     Prints a string with given length using BIOS interrupt 0x10
; 
; PARAMETERS
;     DS:SI - pointer to the string
;     CX    - length of the string
;     
; RETURN
;     None
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Initial version.
; 
PRINTN:
    pusha                   ; Save all general-purpose registers
.next_char:
    cmp cx, 0               ; Check if we've printed enough characters
    je .done
    lodsb                   ; Load byte at [SI] into AL and increment SI
    mov ah, 0x0E            ; BIOS teletype output function
    int 0x10                ; Call BIOS interrupt to print character in AL
    dec cx                  ; Decrement remaining character count
    jmp .next_char
.done:
    popa                    ; Restore all general-purpose registers
    ret


; void PRINT(DS:SI str);
; 
; DESCRIPTION
;     Prints a null-terminated string using BIOS interrupt 0x10
; 
; PARAMETERS
;     DS:SI - pointer to the string
;     
; RETURN
;     None
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Description
; 
PRINT:
    ; save registers
    pusha
print_loop:
    lodsb ; load char to al
    cmp al, 0
    je .print_done
    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp print_loop
.print_done:
    popa
    ret  


; TODO:
;   PRINT_DEC   - Prints input as decimal
;   PRINT_DECN  - Prints input as decimal as long as CX
;
;
;   PRINT_BIN   - Prints input as binary
;   PRINT_BINN  - Prints input as binary as long as CX
;
;   PRINT_HEX   - Prints input as hexadecimal
;   PRINT_HEXN  - Prints input as hexadecimal as long as CX

%endif ; BIOS_IO