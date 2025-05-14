; KERNEL\KERNEL_ENTRY.asm - Kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     16-bit kernel entry point. 
;     This file will load KRNL.BIN to memory, switch to 32-bit protected mode and jump to KRNL.BIN
;
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/09 - Antonako1
;         Initial version. Prints a welcome message.
;     2025/5/11  - Antonako1
;         Transforms into 32-bit mode, sets up memory, idt and gdtr
;     2025/5/14  - Antonako1
;         Jumps to 32-bit kernel
;
; REMARKS
;     None


[BITS 16]
[ORG 0x1000:0x0000]

start:
    mov [drive_number], dl

    mov si, msg_greeting_1
    call PRINTLN

    ; Set up the stack
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Set up the data segment
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    cli

    mov ax, 0
    mov [E820_present], ax
    xor ebx, ebx
    
    ;%%%%%%%%%%%%%%%%%%%%%%%%%
    ; Set up memory
    ;%%%%%%%%%%%%%%%%%%%%%%%%%
.do_mem:
    mov eax, 0xE820
    mov ebx, 0
    lea esi, mem_buf
    mov ecx, MEM_BUF_LEN
    mov edx, SMAP
    int 15h
    jc MEM_ERROR1


    ; Print F-chars of EAX: 0x0000FFFF     
    pusha                   ;1    
        call PRINT_Q
        call PRINT_HEX
        call PRINT_Q
        call PRINT_LINEFEED
        ; Print F-chars of EAX: 0xFFFF0000
        shr eax, 16
        call PRINT_Q
        call PRINT_HEX
        call PRINT_Q
        call PRINT_LINEFEED
        ;%%%%%%%%%%%%%%%%%%%%%%%%
        ; Outputs:
        ;   0x4150
        ;   0x534D
        ; SMAP: 0x534D4150
        ;%%%%%%%%%%%%%%%%%%%%%%%%

        pusha                   ;2
            call PRINT_Q
            mov si, word [eax]
            mov cx, 2
            call PRINTN
            shr eax, 16
            mov si, word [eax]
            mov cx, 2
            call PRINTN
            call PRINT_Q
            call PRINT_LINEFEED
        popa                    ;2

    popa                    ;1


    ; Compares "SMAP" to EAX
    lea di, SMAP_STR
    mov cx, 4
    mov si, [eax]
    call strncmp
    cmp ax, 1
    je MEM_ERROR2


    pusha
    mov eax, ecx
    call PRINT_Q
    call PRINT_HEX
    call PRINT_Q
    call PRINT_LINEFEED
    popa
    
    ; if ecx < 20 goto: MEM_ERROR3
    cmp ecx, 20
    jng MEM_ERROR3

    ; IF ecx > MEM_BUF_LEN goto: MEM_ERROR4
    cmp ecx, MEM_BUF_LEN
    jg MEM_ERROR4
    
    push eax
    mov eax, 1
    mov [E820_present], ax
    pop eax


    ; Add address range Descriptor.BaseAddress through
    ; Descriptor.BaseAddress + Descriptor.Length
    ; as type Descriptor.Type

    ; while ebx != 0 goto: .do_mem
    cmp ebx, 0
    jne .do_mem
    
    ; Read KRNL.BIN from disk, load it into memory and jump to it
    ; The kernel is loaded at 0x100000:0x0000
    ;
    ; Logic copied from bootloader
    
    mov si, msg_kernel_end
    call PRINTLN

    ; Start the 32-bit protected mode
    lgdt [GDTR]    ; load GDT register with start address of Global Descriptor Table
    mov eax, cr0 
    or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax

    lidt [IDTR]

    ; Perform far jump to selector 08h (offset into GDT, pointing at a 32bit PM code segment descriptor) 
    ; to load CS with proper PM32 descriptor)
    jmp 08h:PModeMain

PModeMain:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000     ; Setup 32-bit stack somewhere safe in high memory

    mov eax, 1
    mov [_32_bit_on], eax

KRNL_JMP:

    jmp hang
hang:
    mov ax, [_32_bit_on]
    cmp ax, 1
    je .no_questions
    mov si, msg_hng_1
    call PRINTLN
    
.no_questions:
    jmp $

MEM_ERROR1:
    mov si, msg_mem_err1
    call PRINTLN
    jmp hang
MEM_ERROR2:
    mov si, msg_mem_err2
    call PRINTLN
    jmp hang
MEM_ERROR3:
    mov si, msg_mem_err3
    call PRINTLN
    jmp hang
MEM_ERROR4:
    mov si, msg_mem_err4
    call PRINTLN
    jmp hang
%include "SOURCE/KERNEL/KERNEL_ENTRY_DATA.inc"
%include "SOURCE/KERNEL/16-BIT-BIOS/BIOS.inc"