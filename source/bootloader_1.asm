BITS 16
ORG 0x7c00

%define endl 0x0d, 0x0a

; fat12 header

jmp short start
nop

bdb_oem:                db "MSWIN4.1"
bdb_bytes_per_sector:   dw 512
bdb_sectors_per_cluster:db 1
bdb_reserverd_sectors:  dw 1
bdb_fat_count:          db 2
bdb_dir_entries_count:  dw 0x0e0
bdb_total_sectors:      dw 2880     ;2880+512=1.44mb
mdm_media_descriptor_type:  db 0x0f0
bdb_sectors_per_fat:    dw 9
bdb_sectors_per_track:  dw 18
bdb_heads:              dw 2
bdb_hidden_sectors:     dd 0
bdb_large_sector_count: dd 0

ebr_drive_number:       db 0x00;floppy
                        db 0 ;reserve
ebr_signature:          db 0x29
ebr_volume_id:          db 0x1, 0x88, 0x12, 0x51
ebr_volume_laber:       db "atosrt atos"
ebr_system_id:          db "FAT12   "
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
    mov [ebr_drive_number], dl

    sti
    mov si, msg_loading
    call print

    ; read drive parameters
    push es
    mov ah, 0x08
    int 13h
    jc disk_read_error
    pop es
    
    and cl, 0x3f
    xor ch, ch
    mov [bdb_sectors_per_track], cx ; sector count

    inc dh
    mov [bdb_heads], dh             ; head count

    ; compute lba of root dir (reserved+fats*sectors_per_fat)
    mov ax, [bdb_sectors_per_fat]  
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                          ; ax = fats*sectors_per_fat
    add ax, [bdb_reserverd_sectors] ; ax = lba of root dir
    push ax

    ; compute size of root dir (32*num_of_entries)/bytes_per_sector
    mov ax, [bdb_sectors_per_fat]
    shl ax, 5                       ; ax *= 32
    xor dx, dx                      ; dx = 0
    div word [bdb_bytes_per_sector] ; number of sectors needed to read

    test dx, dx                     ; if dx != 1, add 1
    jz .root_dir_next
    inc ax                          ; div remainder != 0, add 1
    
.root_dir_next:
    ; read root dir
    mov cl, al                      ; cx = number of sectors to read = size of root dir
    pop ax                          ; ax = lba of root dir
    mov dl, [ebr_drive_number]      ; dl = drive number
    mov bx, buffer                  ; es:bx = buffer
    call read_from_disk
    

    ; search for kernel
    xor bx, bx
    mov di, buffer

.search_for_kernel:
    mov si, file_kernel_bin
    mov cx, 11                      ; fat 12 limit is 11 characters
    push di
    repe cmpsb
    pop di
    je .found_kernel
    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_for_kernel

    mov si, msg_kernel_read_failure
    call print
    jmp hang
    
.found_kernel:
    ; di should have address to entry
    mov ax, [di + 26]               ; first logical cluster field
    mov [kernel_cluster], ax
    
    ; load fat from disk into memory
    mov ax, [bdb_reserverd_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call read_from_disk

    ; read kernel and process fat chain

    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    ; reax next cluster
    mov ax, [kernel_cluster]
    add ax, KERNEL_CLUSTER_OFFSET

    mov cl, 1
    mov dl, [ebr_drive_number]
    call read_from_disk

    ; kernel max size 64k
    add bx, [bdb_bytes_per_sector]

    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even
.odd:
    shr ax, 4
    jmp .next_cluster_after
.even:
    and ax, 0x0FFF
.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finish:
    ; boot device in dl
    mov dl, [ebr_drive_number]

    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax
    jmp KERNEL_LOAD_SEGMENT:KERNEL_CLUSTER_OFFSET
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

; SEND IN: (ax: lba address)
; sector number =  (lba % bdb_sectors_per_track) + 1           | cx [bits 0-5]
; head number =    (lba / bdb_sectors_per_track) % bdb_heads   | cx [bits 6-15]
; cylinder number =(lba / bdb_sectors_per_track) / bdb_heads   | dh
lba_chs_conversion:
    push ax
    push dx

    ; calculate sector
    xor dx, dx
    div word [bdb_sectors_per_track]    
        ; ax = lba/bdb_sectors_per_track
        ; dx = lba%bdb_sectors_per_track
    inc dx
    mov cx, dx

    ; calculate cylinder and head
    xor dx, dx
    div word [bdb_heads]
        ; ax = (lba / bdb_sectors_per_track) / bdb_heads 
        ; dx = lba / bdb_sectors_per_track) % bdb_heads
    
    mov dh, dl;dh=head
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax
    ret


; ax: lba address
; cl: number of sectors (max 128)
; dl: drive number
; es:bx: memory address where to store the data
read_from_disk:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx ;will be overwritten in conversion
    call lba_chs_conversion
    pop ax  ;al contains number of sectors to read
    
    mov ah, 0x02
    mov di, 3

.disk_read_retry:
    pusha   ;save all registers
    stc     ;set carry flag
    int 13h
    jnc .disk_read_success

    ; failure
    popa
    call disk_reset
    
    dec di
    test di, di
    jnz .disk_read_retry

.disk_read_fail:
    jmp disk_read_error

.disk_read_success:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; dl: drive_number
disk_reset:
    pusha
    mov ah, 0x0
    stc
    int 0x13
    jc read_from_disk.disk_read_fail
    popa
    ret

msg_loading:            db "Booting", endl, 0
msg_disk_read_failure:  db "Error while reading from disk", endl, 0
msg_kernel_read_failure:  db "Could not find kernel", endl, 0
file_kernel_bin:        db "KERNEL  BIN"
kernel_cluster:         dw 0

KERNEL_LOAD_SEGMENT     equ 0x2000
KERNEL_LOAD_OFFSET      equ 0

KERNEL_CLUSTER_OFFSET   equ 31

times 510-($-$$) db 0
dw 0x0AA55

buffer: