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
;         Transforms into 32-bit mode, sets up idt and gdtr
;     2025/5/15  - Antonako1
;         Sets up memory using E820h
;     2025/5/xx  - Antonako1
;         Jumps to KRNL.BIN;1
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

    ;%%%%%%%%%%%%%%%%%%%%%%%%%
    ; Set up memory
    ;%%%%%%%%%%%%%%%%%%%%%%%%%
    mov byte [E820_present], 0
    xor ebx, ebx
    mov word [num_of_e820_entries], 0

    mov word [e820_entries_ptr], E820_ENTRY_OFFSET
    mov word [e820_entries_ptr+2], E820_ENTRY_SEGMENT

    xor di, di
    mov es, di
.do_mem:
    mov ax, E820R
    shr ax, 4
    mov es, ax

    mov di, E820R
    and di, 0x0F


    mov ecx, MEM_STRUCT_SIZE
    mov edx, SMAP
    mov eax, 0xE820
    int 15h
    jc MEM_ERROR1

    pusha
    call PRINT__
    popa

    ; Compares "SMAP" to EAX
    cmp eax, SMAP
    jne MEM_ERROR2
    
    ; if ecx < MEM_STRUCT_SIZE goto: MEM_ERROR3
    cmp ecx, MEM_STRUCT_SIZE
    jb MEM_ERROR3
    ; IF ecx > MEM_STRUCT_SIZE goto: MEM_ERROR4
    cmp ecx, MEM_STRUCT_SIZE_MAX
    ja MEM_ERROR4

    ; if ecx == MEM_STRUCT_SIZE_MAX; ACPI entry
    ; TODO: ACPI handlings
    
    mov byte [E820_present], 1  ; E820 is present


    ;-----------------------------------
    ; Store entry from mem_buf into E820 table
    ;-----------------------------------
    pusha
    mov eax, [E820R+16]   ; Type
    call PRINT_HEX
    call PRINT_LINEFEED
    popa

;    cmp eax, E820_TYPE_ARM
;    je .ARM
;    cmp eax, E820_TYPE_ARR
;    je .ARR
;    cmp eax, E820_TYPE_ACPI
;    je .ACPI
;    cmp eax, E820_TYPE_NVS
;    je .NVS
;    cmp eax, E820_TYPE_ARU
;    je .ARU
;.ARM:
;.ARR:
;.ACPI:
;.NVS:
;.ARU:

    ; ------------------------------------
    ; 1. Calculate E820 entry linear address
    ; ------------------------------------
    push ebx
    mov bx, [e820_entries_ptr]         ; Offset
    mov ax, [e820_entries_ptr+2]       ; Segment
    shl eax, 4                         ; Segment * 16
    add eax, ebx                       ; Final linear address

    ;mov bx, [e820_entries_ptr]         ; Offset
    ;mov ax, [e820_entries_ptr+2]       ; Segment
    ;shl eax, 4
    ;add eax, ebx                       ; eax = linear address

    ; ------------------------------------
    ; 2. Store the E820 entry at [eax]
    ; ------------------------------------
    
    xor edx, edx
    mov edx, [E820R]                  
    mov [eax], edx                    ; BaseAddressLow
    mov edx, [E820R+4]
    mov [eax+4], edx                  ; BaseAddressHigh
    mov edx, [E820R+8]
    mov [eax+8], edx                  ; LengthLow
    mov edx, [E820R+12]
    mov [eax+12], edx                 ; LengthHigh
    mov edx, dword [E820R+16]
    mov [eax+16], edx                 ; Type

%ifdef E820_ACPI
    mov edx, [E820R+20]
    mov [eax+20], edx                 ; ACPI (optional)
%endif


    ; ------------------------------------
    ; 3. Advance e820_entries_ptr (segment:offset)
    ; ------------------------------------
    add eax, MEM_STRUCT_SIZE          ; move to next slot
    mov ebx, eax                      ; Copy for splitting
    and bx, 0x000F                    ; new offset = linear & 0xF
    shr eax, 4                        ; new segment = linear >> 4
    mov [e820_entries_ptr], bx        ; Store new offset
    mov [e820_entries_ptr+2], ax      ; Store new segment
    
    pop ebx

    inc word [num_of_e820_entries]

    
    ; while ebx != 0 goto: .do_mem
    cmp ebx, 0
    jne .do_mem
;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


    ; If E820 is not present, use AX=E801h
    cmp byte [E820_present], 1
    jne .TRY_E801h

    ; Print the E820 entries
    mov ax, word [num_of_e820_entries]
    call PRINT_DEC
    call PRINT__
    call PRINT_LINEFEED



    ; reset E820 entry pointer
    mov word [e820_entries_ptr], E820_ENTRY_OFFSET
    mov word [e820_entries_ptr+2], E820_ENTRY_SEGMENT




    xor ecx, ecx
    mov cx, [num_of_e820_entries]

.loop_print_e820:
    ; --------------------------------------
    ; 1. Calculate linear address of entry
    ; --------------------------------------
    mov bx, [e820_entries_ptr]         ; Offset
    mov ax, [e820_entries_ptr+2]       ; Segment
    shl eax, 4                         ; Segment * 16
    add eax, ebx                       ; Final linear address
    
    ; --------------------------------------
    push eax
    mov ebx, eax                      ; Copy for splitting
    mov eax, [ebx+16]                ; Type field
    call PRINT_HEX
    call PRINT_LINEFEED
    pop eax
    ; --------------------------------------
    ; 3. Advance to next entry
    add eax, MEM_STRUCT_SIZE
    mov ebx, eax
    and bx, 0x000F                     ; offset
    shr eax, 4                         ; segment
    mov [e820_entries_ptr], bx
    mov [e820_entries_ptr+2], ax

    loop .loop_print_e820              ; dec cx / jnz loop

    ; reset E820 entry pointer
    mov [e820_entries_ptr], ax
    mov [e820_entries_ptr+2], dx
    jmp .FIND_KERNEL

.TRY_E801h:
    ;TODO.
    ; If E801h is not present, use AH=88h
    cmp byte [E801_present], 0
    jne .FIND_KERNEL
.TRY_AH88h:
    ;TODO.

.FIND_KERNEL:
    ; Read KRNL.BIN from disk, load it into memory and jump to it
    ; The kernel is loaded at 0x100000:0x0000
    ;
    ; Logic copied from bootloader. Status saved in krnl_status
    
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