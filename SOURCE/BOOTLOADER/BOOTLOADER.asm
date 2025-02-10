; BOOTLOADER\BOOTLOADER.asm - Bootloader for the OS
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Bootloader for the OS. This is the first code that runs when the computer starts
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/02/06 - Antonako1
;         FAT16 Boot Sector Header and welcome message
;     2025/02/09 - Antonako1
;         Removed FAT16 Boot sector for ISO9660 header
;     2025/02/10 - Antonako1
;         Added kernel loading code
; REMARKS
;     None


%define ISO9660_MAGIC_1         0x4344
%define ISO9660_MAGIC_2         0x3030
%define ISO9660_MAGIC_3         31 ; CD001

%define DAP_MEMORY_OFFSET       0x2000      ; Destination offset in memory

; SECTOR_SIZE * VOL_DESC_START = 16 * 2048 = 32KB. Volume descriptor starts at 32KB
%define SECTOR_SIZE             2048        ; 2KB per sector
%define VOL_DESC_START          16          ; Volume descriptor starts at sector 16
%define MAX_PATH                256         ; Maximum path length
%define KERNEL_LOAD_ADDRESS     0x1000      ; Kernel load address inside its segment
%define ROOT_DIRECTORY_OFFSET   0x9C        ; Root directory offset in the ISO9660 image
%define ROOT_DIRECTORY_SIZE     0x22        ; Size of the root directory in sectors

[ORG 0x7C00]

start:
    cli                      ; Clear interrupts
    xor ax, ax               ; Zero out registers
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00           ; Stack at end of bootloader

    mov si, bootMessage
    call print_string

    ; Read the first sector of the CD-ROM
    mov si, dap
    mov dl, 0xE0             ; CD-ROM drive
    mov ah, 0x42             ; Extended read
    int 0x13
    jc disk_error


    ; Read PVD
    mov si, dap
    mov dl, 0xE0
    mov ah, 0x42
    mov al, 1
    mov bx, 16
    int 0x13
    jc disk_error

    mov al, [DAP_MEMORY_OFFSET]          ; Descriptor type (should be 1 for Primary Volume Descriptor)
    cmp al, 1                 ; Compare with expected value
    jne not_iso9660           ; If not 1, it's not a valid ISO

    ; Read root directory
    mov si, dap
    mov dl, 0xE0
    mov ah, 0x42
    mov al, 1
    mov bx, ROOT_DIRECTORY_OFFSET
    int 0x13
    jc disk_error

    ; Step 4: Parse root directory and find "KERNEL.BIN"
    ; (You'll need to write code for parsing directory entries)

    ; Step 5: Read the kernel (once you find it)
    ; (After finding the kernel's entry, read it into memory)

    ; Step 6: Jump to the kernel
    jmp KERNEL_LOAD_ADDRESS

    jmp $

disk_error:
    mov si, diskErrorMessage
    call print_string
    jmp $

not_iso9660:
    mov si, ISO9660ErrorMessage
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
    .done:
        ret

bootMessage         db "Booting atOS revised technology...", 0
diskErrorMessage    db "Error reading disk", 0
ISO9660ErrorMessage db "Not an ISO9660 image", 0
kernelFile          db "KERNEL.BIN", 0
offset              dd 0 ; Offset of current directory
size                dd 0 ; Size of current directory
lba                 dd 0 ; LBA of current directory

; Disk Access Packet (DAP)
dap:
    db 0x10         ; Size of DAP (must be 16 bytes)
    db 0            ; Reserved, must be 0
    dw 1            ; Number of sectors to read
    dw DAP_MEMORY_OFFSET       ; Destination offset in memory
    dw 0            ; Destination segment in memory (0 for real mode)
    dd 0            ; LBA lower 32 bits
    dd 0            ; LBA upper 32 bits (ignored in most cases)

times 510-($-$$) db 0
dw 0xAA55
