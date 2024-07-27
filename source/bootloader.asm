BITS 16

; boot0 initialization

disk_read_time db 0x0

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    jmp short canonicalized ;set cs and ip
canonicalized:
    mov ax, 0x07C0  ; Set data segment to where BIOS loads us
    mov ds, ax
    mov es, ax

    sti

    mov ah, 0x00
    int 0x13
    jc disk_reset_error


; if carry flag is set, increment
; disk_read_time by one
; if disk_read_time == 3
; log error message
reading_disk:
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 1
    mov dh, 0
    mov dl, 0x00

    mov ax, 0x1000
    mov es, ax
    mov bx, 0x0000
    
    int 0x13
    jc error_reading_disk
    jmp continue_from_disk

error_reading_disk:
    mov al, [disk_read_time]
    inc al
    mov [disk_read_time], al
    cmp al, 3
    je disk_read_error
    jmp reading_disk
    
continue_from_disk:
    ; disk buffer loaded to ah



    mov si, msg     ; Load address of message
    call print_string
    jmp hang_cli



;boot1 initialization


disk_read_error:
    mov si, disk_read_msg
    call print_string
    jmp hang_cli

disk_reset_error:
    mov si, disk_reset_msg
    call print_string
    jmp hang_cli
    
print_string:
    lodsb               ; Load byte at [si] into al and increment si
    cmp al, 0           ; Check if we reached the end of the string
    je print_string_done
    mov ah, 0x0E        ; BIOS teletype function
    mov bh, 0x00        ; Page number
    mov bl, 0x07        ; Text attribute (white on black)
    int 0x10            ; BIOS video interrupt
    jmp print_string    ; Loop back to print next character
print_string_done:
    ret


hang_cli:
    cli
    jmp hang

hang:
    jmp hang

disk_reset_msg db "Error resetting disk", 0
disk_read_msg db "Error reading disk", 0
msg db 'Hello, World!', 0

times 510-($-$$) db 0
dw 0xAA55   ; Boot signature

