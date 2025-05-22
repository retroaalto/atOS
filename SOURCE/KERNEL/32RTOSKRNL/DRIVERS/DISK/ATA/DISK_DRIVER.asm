; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\DISK\ATA\DISK_DRIVER.asm - ATA PIO Disk Driver
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     ATA PIO Disk Driver for 32RTOSKRNL.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Created this file with ATA PIO Identify,
;         ATA PIO Read Sectors and ATA PIO Write Sectors functions.
; 
; REMARKS
;     Only to be used by the kernel.
[BITS 32]
%ifndef ATA_DISK_DRIVER_ASM
%define ATA_DISK_DRIVER_ASM
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATA/DISK_DRIVER.inc"
%include "SOURCE/STD/STACK.inc"
%include "SOURCE/STD/CPU_IO.inc"

ATA_PIO_IDENTIFY_MASTER:
    mov bl, ATA_MASTER
    call ATA_PIO_IDENTIFY
    ret
ATA_PIO_IDENTIFY_SLAVE:
    mov bl, ATA_SLAVE
    call ATA_PIO_IDENTIFY
    ret

; bl=0xA0 for master, 0xB0 for slave
; returns: eax=1 if success, 0 if error
; ebp-2 = status
; ebp-4 = mid
; ebp-6 = low
; ebp-10 = index
; ebp-14 = buff
ATA_PIO_IDENTIFY:
    CREATE_STACK 10+(256*2)
    ; U16 [ebp-2] = status
    ; U16 [ebp-4] = mid
    ; U16 [ebp-6] = low
    ; U32 [ebp-10] = index
    ; U512 [ebp-14] = buff
    outb    ATA_PRIMARY_DRIVE_HEAD,     bl
    outb    ATA_PRIMARY_SECCOUNT,       byte 0x00
    outb    ATA_PRIMARY_LBA_LO,         byte 0x00
    outb    ATA_PRIMARY_LBA_MID,        byte 0x00
    outb    ATA_PRIMARY_LBA_HI,         byte 0x00
    outb    ATA_PRIMARY_COMM_REGSTAT,   byte 0xEC
    inb ATA_PRIMARY_COMM_REGSTAT
    mov byte [ebp-2], al

    cmp byte [ebp-2], 0x00
    jz .return_false
    ; outb    ATA_PRIMARY_COMM_REGSTAT,   byte 0xE7

    test byte [ebp-2], STAT_BSY
    jnz .wait_for_status
    jmp .status_done
.wait_for_status:
    
    mov dword [ebp-10], 0
.delay_loop:
    inc dword [ebp-10]
    cmp dword [ebp-10], 0x0FFFFFFF
    jl .delay_loop


    inb ATA_PRIMARY_COMM_REGSTAT
    mov byte [ebp-2], al
    test byte [ebp-2], STAT_BSY
    jnz .wait_for_status
.status_done:

    cmp word [ebp-2], 0x0
    jz .return_false

.poll:
    inb ATA_PRIMARY_COMM_REGSTAT
    mov byte [ebp-2], al
    test byte [ebp-2], STAT_BSY
    jnz .poll
.poll_done:
     inb ATA_PRIMARY_LBA_MID

    mov byte [ebp-4], al
    inb ATA_PRIMARY_LBA_HI
    mov byte [ebp-6], al

    cmp byte [ebp-4], 0x00
    jnz .return_false
    cmp byte [ebp-6], 0x00
    jnz .return_false



.ERR_DRQ_WAIT:
    inb ATA_PRIMARY_COMM_REGSTAT
    mov byte [ebp-2], al
    test byte [ebp-2], (STAT_ERR | STAT_DRQ)
    jz .ERR_DRQ_WAIT


    test byte [ebp-2], STAT_ERR
    jnz .return_false

    mov dx, ATA_PRIMARY_DATA
    mov edi, [ebp-14]
    mov ecx, 256
    rep insw


.return_true:
    mov eax, 1
    CLEAN_STACK
    ret
.return_false:
    xor eax, eax
    CLEAN_STACK
    ret


%endif ; ATA_DISK_DRIVER_ASM