; BOOTLOADER\BOOTLOADER.asm
; Author:     Antonako1
; Date:       2025-02-06
; Version:    1.0
; Description: Bootloader for the OS
;              Loads the kernel from the disk and jumps to it
; DATE: 2025-02-06
;   - Initial version
;       - FAT16 Boot Sector Header and welcome message
; DATE: 2025-02-07
;   - Added

[ORG 0x7C00]

; FAT16 Boot Sector Header (BIOS Parameter Block - BPB)
jmp short start                     ; Jump to bootloader code
nop                                 ; Padding (required for FAT16)
OEMLabel       		db "MSWIN4.1"   ; 8-byte OEM name

; BIOS Parameter Block (BPB) for FAT16
BytesPerSector 		dw 512       	; Sector size (512 bytes)
SectorsPerCluster 	db 1      	    ; 1 sector per cluster
ReservedSectors 	dw 1        	; 1 reserved sector (boot sector)
NumberOfFATs 		db 2           	; Number of FAT tables
RootEntries 		dw 224          ; Max root directory entries
TotalSectors16 		dw 2880      	; Total sectors (for floppy disk)
MediaDescriptor 	db 0xF8     	; Media descriptor (Hard disk)
SectorsPerFAT16 	dw 9        	; FAT16 size (sectors per FAT)
SectorsPerTrack 	dw 18       	; 18 sectors per track
NumberOfHeads 		dw 2          	; 2 heads
HiddenSectors 		dd 0          	; No hidden sectors
TotalSectors32 		dd 0         	; Used only for FAT32 (set to 0 for FAT16)

; Extended Boot Record
DriveNumber 		db 0x80         ; Drive number (0x00 = floppy, 0x80 = HDD)
Reserved 		    db 0            ; Reserved
Signature 		    db 0x29         ; Extended boot record signature
VolumeID 		    dd 0x12345678   ; Volume serial number
VolumeLabel 		db "BOOTLOADER "; 11-byte volume label
FileSystemType 		db "FAT16   " 	; 8-byte file system type

; Bootloader Code
start:
    cli                      ; Clear interrupts
    xor ax, ax               ; Zero out registers
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00           ; Stack at end of bootloader

    mov si, bootMessage
    call print_string

    ; halt
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done:
    ret

bootMessage db "Booting atOS revised technology...", 0

times 510-($-$$) db 0
dw 0xAA55
