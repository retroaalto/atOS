; Prints a string with given length using BIOS interrupt 0x10
; Inputs:
;   DS:SI - pointer to the string
;   CX    - length of the string
bios_print:
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