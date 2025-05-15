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
;     Functions:
;       PRINTN - Prints a string with given length 
;       PRINT  - Prints a null-terminated string
;       PRINTN_Q - Prints a string with given length
;       PRINTNLN - Prints a string with given length
;       PRINTLN - Prints a null-terminated string
;       PRINT_LINEFEED - Prints a line break (CR + LF)
;       PRINT_DEC - Prints input as decimal
;       PRINT_DECN - Prints input as decimal as long as CX
;       PRINT_BIN - Prints input as binary
;       PRINT_BINN - Prints input as binary as long as CX
;       PRINT_HEX - Prints input as hexadecimal
;       PRINT_HEXN - Prints input as hexadecimal as long as CX
;       PRINT_Q - Prints a double quote character
;       PRINT_HEX_N_RANGE - Prints CX bytes at address EAX as hex (with "0x" prefix)
;       PUTCHAR - Prints a character
;       PUTCHARLN - Prints a character and adds \r\n at the end
;       PRINT__ - Prints a underscore character
;

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

; void PRINTN_Q(DS:SI str, cx len);
;
;
; DESCRIPTION
;     Prints a string with given length using BIOS interrupt 0x10
;     and adds quotes around it.
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
PRINTN_Q:
    pusha                   ; Save all general-purpose registers

    pusha
    mov al, '"'
    mov ah, 0x0E
    int 0x10
    popa 
    
.next_char:
   cmp cx, 0               ; Check if we've printed enough characters
   je .close_quote          ; If CX is 0, jump to print closing quote
   lodsb                   ; Load byte at [SI] into AL and increment SI    
   mov ah, 0x0E            ; BIOS teletype output function
   int 0x10                ; Call BIOS interrupt to print character in AL
   dec cx                  ; Decrement remaining character count
   jmp .next_char          ; Continue printing the next character
   
.close_quote:
    ; Print closing single quote
    mov al, '"'
    mov ah, 0x0E
    int 0x10
    popa                    ; Restore all general-purpose registers
    ret

PRINT_LINEFEED:
    pusha                   ; Save all general-purpose registers
    ; Print line break (CR + LF)
    mov al, 0x0D            ; Carriage return
    mov ah, 0x0E
    int 0x10
    mov al, 0x0A            ; Line feed
    mov ah, 0x0E
    int 0x10
    popa                    ; Restore all general-purpose registers
    ret

; void PRINTNLN(DS:SI str, cx len);
;
; DESCRIPTION
;     Prints a string with given length using BIOS interrupt 0x10
;     and adds \r\n at the end.
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
PRINTNLN:
    call PRINTN            ; Call PRINTN to print the string
    call PRINT_LINEFEED ; Call PRINT_LINEFEED to print CR + LF
    ret

; void PRINTLN(DS:SI str);
;
; DESCRIPTION
;     Prints a null-terminated string using BIOS interrupt 0x10
;     and adds \r\n at the end.
;
; PARAMETERS
;     DS:SI - pointer to the string
;
; RETURN
;     None
;
; AUTHORS
;     Antonako1
PRINTLN:
    call PRINT            ; Call PRINTN to print the string
    ; Print line break (CR + LF)
    call PRINT_LINEFEED ; Call PRINT_LINEFEED to print CR + LF
    ret

PRINTNLN_Q:
    call PRINTN_Q
    call PRINT_LINEFEED
    ret

; void PUTCHAR(AL char);
;
; DESCRIPTION
;     Prints a character
;
; PARAMETERS
;     AL - character to print
;
; RETURN
;     None
PUTCHAR:
    pusha
    mov ah, 0x0E
    int 0x10
    popa
    ret

; void PUTCHARLN(AL char);
;
; DESCRIPTION
;     Prints a character and adds \r\n at the end.
;
; PARAMETERS
;     AL - character to print
;
; RETURN
;     None
PUTCHARLN:
    pusha
    call PUTCHAR ; Call PUTCHAR to print the character
    call PRINT_LINEFEED ; Call PRINT_LINEFEED to print CR + LF
    popa
    ret

PRINT__:
    pusha
    mov al, '_'
    call PUTCHAR
    popa
    ret 
    
; TODO:
;   PRINT_DEC   - Prints input as decimal
;   PRINT_DECN  - Prints input as decimal as long as CX

;   void PRINT_DEC(AX);
;
; DESCRIPTION
;     Prints the value in AX as decimal using BIOS interrupt 0x21
;     and INT 0x10.
;     The value is printed in 8 digits (32 bits).
; PARAMETERS
;     EAX - value to print
; RETURN
;     None
PRINT_DEC:
    pusha

    add eax, 0x30
    call PUTCHAR

    popa
    ret


;   PRINT_BIN   - Prints input as binary
;   PRINT_BINN  - Prints input as binary as long as CX


; void PRINT_HEXN(EAX, CX)
;
; Prints EAX as CX hex digits (e.g., CX = 8 for 32-bit, CX = 4 for 16-bit, etc.)
; Shows "0x" prefix, followed by big-endian hex digits.
;
PRINT_HEXN:
    pusha

    ; Show "0x"
    mov al, '0'
    call PUTCHAR
    mov al, 'x'
    call PUTCHAR

    mov ebx, eax       ; Copy the value to EBX to preserve EAX
    mov esi, ecx        ; Save CX in ESI for loop control

.next_nibble:
    dec esi
    mov ecx, esi
    shl ecx, 2         ; Multiply index by 4 to shift to correct nibble
    mov eax, ebx
    shr eax, cl        ; Bring desired nibble into low 4 bits
    and al, 0x0F       ; Isolate low nibble

    ; Convert nibble to ASCII
    cmp al, 9
    jbe .digit
    add al, 7
.digit:
    add al, '0'
    call PUTCHAR

    cmp esi, 0
    jne .next_nibble

    popa
    ret


;  void PRINT_HEX(AX);
;
; DESCRIPTION
;     Prints the value in AX as hexadecimal using BIOS interrupt 0x21
;     and INT 0x10.
;     The value is printed in 8 digits (32 bits).
; PARAMETERS
;     EAX - value to print
; RETURN
;     None
PRINT_HEX:
    pusha

    push eax
    mov al, '0'
    call PUTCHAR
    mov al, 'x'
    call PUTCHAR
    pop eax

    ; Move AX to a working register so we don't destroy it
    mov cx, 8             ; 4 nibbles (16 bits)
    mov ebx, eax            ; Copy AX into BX

.print_hex_loop:
    rol ebx, 4             ; Rotate left 4 bits to bring next nibble into low nibble
    mov dl, bl            ; Get low 8 bits
    and dl, 0x0F          ; Isolate the lowest nibble
    cmp dl, 9
    jbe .print_digit
    add dl, 7             ; Adjust for 'A'-'F'

.print_digit:
    add dl, '0'
    push eax
    mov al, dl
    call PUTCHAR
    pop eax
    loop .print_hex_loop

    popa
    ret

; void PRINT_Q(void)
;
; DESCRIPTION
;     Prints a double quote character using BIOS interrupt 0x10
;
; PARAMETERS
;     None
; RETURN
;     None
PRINT_Q:
    pusha
    mov al, '"'
    call PUTCHAR
    popa
    ret


; PRINT_HEX_N_RANGE:
;   Prints CX bytes at address EAX as hex (with "0x" prefix)
;   Example: if [EAX] = 0x12 0xAB, it prints: 0x12AB
;
; INPUT:
;   EAX = pointer to memory
;   CX  = number of bytes to print
;
; PRINT_HEX_N_RANGE:
;     pusha

;     ; Print "0x" prefix
;     mov al, '0'
;     call PUTCHAR
;     mov al, 'x'
;     call PUTCHAR

;     ; Print each byte from memory
;     mov esi, eax        ; ESI = pointer to memory

; .next_byte:
;     mov al, [esi]       ; Load byte from memory
;     call PRINT_BYTE_HEX
;     inc esi
;     loop .next_byte     ; CX is already our counter

;     popa
;     ret

; ; Helper function: Print AL as 2-digit hex (e.g., AL=0xAB -> prints "AB")
; PRINT_BYTE_HEX:
;     push ax
;     push bx

;     mov ah, al
;     shr ah, 4
;     and ah, 0x0F
;     call HEX_NIBBLE_PRINT

;     and al, 0x0F
;     call HEX_NIBBLE_PRINT

;     pop bx
;     pop ax
;     ret

; ; Print nibble in AL (must be 0-15)
; HEX_NIBBLE_PRINT:
;     cmp al, 9
;     jbe .digit
;     add al, 7
; .digit:
;     add al, '0'
;     call PUTCHAR
;     ret

%endif ; BIOS_IO