BITS 16
ORG 0x7c00

; fat32 header start
; http://elm-chan.org/docs/fat_e.html#notes
jmp short start
nop
; bios parameter block (bdb)
bdb_oem:                db "MSWIN4.1"   ; 8 byte min length
bdb_bytes_per_sector:   dw 512
bdb_sectors_per_cluster:db 8
bdb_reserverd_sectors:  dw 32           ; precedes first FAT, including boot sector
bdb_fat_count:          db 2            ; don't change
bdb_root_entry_count:   dw 0            ; don't change
bdb_total_sectors:      dw 0            ; don't change
mdm_media_descriptor_type:  db 0xF8     ; 0xF0 for floppy disks, 0xF8 for hard disk
bdb_sectors_per_fat:    dw 0            ; don't change
bdb_sectors_per_track:  dw 63           ; 18 floppy
bdb_heads:              dw 255
bdb_hidden_sectors:     dd 0
bdb_large_sector_count: dd 0

; extended boot record
erb_sectors_per_fat:    dd 0
erb_flags:              dw 0
erb_fat_version_num:    dw 0
erb_cluster_number_of_root_directory:   dd 2
erb_fsinfo_structure:   dw 1
erb_backup_boot_sector: dw 6
                        db 0            ; Reserved (12 bytes)
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved
                        db 0            ; Reserved

; extended bios parameter block
ebpb_drive_number:      db 0x80 ;floppy 0x00, 0x80 first hard drive or cd-rom
                        db 0    ;reserve
ebpb_signature:         db 0x29
ebpb_volume_id:         dd 0x51128801
ebpb_volume_laber:      db "ATOS 2.0/RT" ; 11 bytes
ebpb_system_id:         dq "FAT32   "
; fat32 header end

start:
    jmp boot0

boot0:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7c00

    push es
    push word .next
    retf
.next:

    ; mov [ebpb_drive_number], dl

    sti
    mov si, msg_loading
    call print

    jmp hang
    
    
hang:
    cli
    hlt





; ds:si points to a string
print:
    ; save registers
    push si
    push ax
print_loop:
    lodsb ; load char to al
    cmp al, 0
    je .print_done
    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp print_loop
.print_done:
    pop ax
    pop si
    ret   

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;       DISK FUNCTIONS
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

disk_read_error:
    mov si, msg_disk_read_failure
    call print
    jmp hang



; dl: drive_number
disk_reset:
    pusha
    mov ah, 0x0
    stc
    int 0x13
    jc disk_read_error ; TODO ADD A . directive
    popa
    ret

msg_loading:            db "Booting", 0
msg_disk_read_failure:  db "Error while reading from disk", 0
msg_kernel_read_failure:db "Could not find kernel", 0
file_kernel_bin:        db "KERNEL  BIN"
kernel_cluster:         dw 0

KERNEL_LOAD_SEGMENT     equ 0x2000
KERNEL_LOAD_OFFSET      equ 0

KERNEL_CLUSTER_OFFSET   equ 31

times 510-($-$$) db 0
dw 0x0AA55
