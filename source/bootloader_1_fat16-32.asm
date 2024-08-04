BITS 16
ORG 0x7c00


; fat header start
; TODO ADD PREPROCESSOR TO CHECK FAT TYPE
; http://elm-chan.org/docs/fat_e.html#notes
jmp short start
nop
; bios parameter block (bdb)
bdb_oem:                db "MSWIN4.1"   ; 8 bytes
bdb_bytes_per_sector:   dw 512          ; DON'T CHANGE
bdb_sectors_per_cluster:db 1
bdb_reserverd_sectors:  dw 1
;FAT32 bdb_reserverd_sectors:  dw 32           ; precedes first FAT, including boot sector
bdb_fat_count:          db 2            ; don't change
bdb_root_entry_count:   dw 0x0E0
;FAT32 bdb_root_entry_count:   dw 0            ; don't change
bdb_total_sectors:      dw 2880
;FAT32 bdb_total_sectors:      dw 0            ; don't change
mdm_media_desc_type:    db 0xF0         ; 0xF0 for floppy disks, 0xF8 for hard disk
bdb_sectors_per_fat:    dw 9
;FAT32 bdb_sectors_per_fat:    dw 0            ; don't change
bdb_sectors_per_track:  dw 18
;FAT32 bdb_sectors_per_track:  dw 63           ; 18 floppy
bdb_heads:              dw 2
;FAT32 bdb_heads:              dw 255
bdb_hidden_sectors:     dd 0
bdb_large_sector_count: dd 0


;FAT32 extended boot record
;FAT32 erb_sectors_per_fat:    dd 0
;FAT32 erb_flags:              dw 0
;FAT32 erb_fat_version_num:    dw 0
;FAT32 erb_clust_num_root_dir: dd 2
;FAT32 erb_fsinfo_structure:   dw 1
;FAT32 erb_backup_boot_sector: dw 6
;FAT32                         db 0            ; Reserved (12 bytes)
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved
;FAT32                         db 0            ; Reserved

; extended bios parameter block
ebpb_drive_number:      db 0x80 ;floppy 0x00, 0x80 first hard drive or cd-rom
                        db 0    ;reserve
ebpb_signature:         db 0x29
ebpb_volume_id:         db  0x53, 0x4F, 0x54, 0x41
ebpb_volume_laber:      db "_ATOS2.0|RT" ; 11 bytes

ebpb_system_id:         dq "FAT16   " ; File system type (8 bytes)
;FAT32 ebpb_system_id:         dq "FAT32   " ; File system type (8 bytes)
; fat header end

start:
    jmp boot0

boot0:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7c00

    ; set cs and ip
    push es
    push word .next
    retf
.next:
    ; dl has drive number. saved from bios
    mov [ebr_drive_number], dl

    sti
    mov si, msg_boot
    call print

    ; read drive parameters
    push es
    mov ah, 0x08
    int 13h
    ; AH: Status of the operation. If AH = 0x00, the operation was successful.
    ; CH: Low 8 bits of the number of cylinders.
    ; CL: Bits 6 and 7 are high bits of the number of cylinders. Bits 0-5 are the number of sectors per track.
    ; DH: Number of heads.
    ; DL: Number of drives.
    ; ES: Address of the drive parameter table (if applicable).
    jc disk_read_error
    pop es

    
    jmp hang
    
    
hang:
    cli
    hlt





; ds:si points to a string
print:
    push si;
    push ax;
print_loop:
    lodsb ; load char to al
    cmp al, 0;
    je .print_done;
    mov ah, 0x0e;
    mov bh, 0;
    int 0x10;
    jmp print_loop;
.print_done:
    pop ax;
    pop si;
    ret;

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;       DISK FUNCTIONS
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

disk_read_error:
    mov si, msg_disk_read_error;
    call print;
    jmp hang;


msg_boot:            db "Booting", 0;
msg_disk_read_error: db "Disk read error", 0;

times 510-($-$$) db 0;
dw 0x0AA55;
