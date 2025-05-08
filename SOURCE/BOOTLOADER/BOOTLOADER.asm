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
BITS 16
[ORG 0x7C00]

%define PVD_OFFSET              16*2048/512
%define DAP_MEMORY_OFFSET       0x2000      ; Destination offset in memory

; SECTOR_SIZE * VOL_DESC_START = 16 * 2048 = 32KB. Volume descriptor starts at 32KB
%define SECTOR_SIZE             2048        ; 2KB per sector
%define VOL_DESC_START          16          ; Volume descriptor starts at sector 16

%define ROOT_DIRECTORY_OFFSET   0x9C        ; Root directory offset in the ISO9660 image
%define ROOT_DIRECTORY_SIZE     0x22        ; Size of the root directory in sectors

%define MAX_PATH                256         ; Maximum path length
KERNEL_LOAD_ADDRESS equ 0x1000

; read sector of 16*2048 (lba chs conversion)
; -> pvd at the start of this sector
; -> read KERNEL.BIN to memory


start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [drive_number], dl

    sti
    mov si, bootMessage
    call print_string

    ; Set up the Disk Address Packet (DAP)
    mov si, dap
    mov byte [si], 0x10         ; Size of DAP (16 bytes)
    mov byte [si+1], 0          ; Reserved
    mov word [si+2], 1          ; Number of sectors to read
    mov word [si+4], buffer     ; Offset
    mov word [si+6], 0x0000     ; Segment
    mov dword [si+8], 64        ; LBA low
    mov dword [si+12], 0        ; LBA high (not used)

    mov ah, 0x42                ; Extended Read
    mov dl, [drive_number]
    int 0x13
    jc disk_error

    jmp bootend
    
bootend:
    mov ax, 0xb800
    mov es, ax
    mov word [es:0x0000], 0x57<<8 | 'A'
    jmp $

disk_error:
    mov si, diskErrorMessage
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

; Messages
bootMessage         db "Booting atOS revised technology...", 0
diskErrorMessage    db "E_R_D", 0

drive_number        db 0

; Disk Address Packet (DAP)
dap:
    times 16 db 0

; Align boot sector
times 510-($-$$) db 0
dw 0xAA55

buffer:
