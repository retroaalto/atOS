; KERNEL\KERNEL_ENTRY.asm - Kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     16-bit kernel entry point. 
;     This file will switch to 32-bit protected mode and load the master kernel file.
;
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/09 - Antonako1
;         Initial version. Prints a welcome message.
;     2025/5/11  - Antonako1
;         Transforms into 32-bit mode, sets up memory, idt and gdtr
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
    ; Set up memory
    ; INT 0x15, AX=0xE820
    
    ; Set up IDT to use BIOS calls



    mov si, msg_kernel_end
    call PRINTLN

    ; Start the 32-bit protected mode
    lgdt [GDTR]    ; load GDT register with start address of Global Descriptor Table
    mov eax, cr0 
    or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax

    ; Perform far jump to selector 08h (offset into GDT, pointing at a 32bit PM code segment descriptor) 
    ; to load CS with proper PM32 descriptor)
    jmp 08h:PModeMain

PModeMain:
    ; load DS, ES, FS, GS, SS, ESP
    mov ax, 0x10    ; data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, 0x7c00
    mov esp, eax
    
KRNL_JMP:
    ; Read KRNL.BIN from disk, load it into memory and jump to it
    ; The kernel is loaded at 0x100000:0x0000

    jmp hang
hang:
    cli
    hlt


%include "SOURCE/KERNEL/KERNEL_ENTRY_DATA.inc"
%include "SOURCE/KERNEL/16-BIT-BIOS/BIOS.inc"