; SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATA_SHARED.inc - ATA disk driver include file
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the definitions and constants used by the ATA/ATAPI disk driver.
;
; AUTHORS
;     Antonako1
;
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Initial version. Contains basic ports and register definitions.
;
; REMARKS
;     Only to be used by the kernel and disk drivers.
%ifndef ATA_SHARED_INC
%define ATA_SHARED_INC
%include "SOURCE/STD/CPU_IO.inc"
%include "SOURCE/STD/STACK.inc"

;-------------------------
; Ports
;-------------------------
ATA_DATA EQU 0
ATA_ERR  EQU 1
ATA_SECCOUNT EQU 2
ATA_LBA_LO EQU 3
ATA_LBA_MID EQU 4
ATA_LBA_HI EQU 5
ATA_DRIVE_HEAD EQU 6
ATA_COMM_REGSTAT EQU 7

ATA_BASE           EQU 0x1F0 ; Base port for ATA
ATA_PRIMARY_DATA    EQU ATA_BASE+ATA_DATA; Primary ATA port
ATA_PRIMARY_ERR      EQU ATA_BASE+ATA_ERR ; Error register
ATA_PRIMARY_SECCOUNT        EQU ATA_BASE+ATA_SECCOUNT  ; Sector count register
ATA_PRIMARY_LBA_LO    EQU ATA_BASE+ATA_LBA_LO ; LBA low register
ATA_PRIMARY_LBA_MID    EQU ATA_BASE+ATA_LBA_MID ; LBA mid register
ATA_PRIMARY_LBA_HI   EQU ATA_BASE+ATA_LBA_HI ; LBA high register
ATA_PRIMARY_DRIVE_HEAD EQU ATA_BASE+ATA_DRIVE_HEAD ; Drive/Head register
ATA_PRIMARY_COMM_REGSTAT   EQU ATA_BASE+ATA_COMM_REGSTAT ; Status register
ATA_PRIMARY_ALTSTAT_DCR  EQU 0x3F6 ; Alternate status register

ATA_SECOND_BASE   EQU 0x170 ; Base port for secondary ATA
ATA_SECONDARY_DATA    EQU ATA_SECOND_BASE +ATA_DATA; Secondary ATA port
ATA_SECONDARY_ERR      EQU ATA_SECOND_BASE+ATA_ERR ; Error register
ATA_SECONDARY_SECCOUNT        EQU ATA_SECOND_BASE+ATA_SECCOUNT  ; Sector count register
ATA_SECONDARY_LBA_LO    EQU ATA_SECOND_BASE+ATA_LBA_LO ; LBA low register
ATA_SECONDARY_LBA_MID    EQU ATA_SECOND_BASE+ATA_LBA_MID ; LBA mid register
ATA_SECONDARY_LBA_HI   EQU ATA_SECOND_BASE+ATA_LBA_HI ; LBA high register
ATA_SECONDARY_DRIVE_HEAD EQU ATA_SECOND_BASE+ATA_DRIVE_HEAD ; Drive/Head register
ATA_SECONDARY_COMM_REGSTAT   EQU ATA_SECOND_BASE+ATA_COMM_REGSTAT ; Status register
ATA_SECONDARY_ALTSTAT_DCR  EQU 0x376 ; Alternate status register

;-------------------------
; Commands
;-------------------------
ATAPI_PACKET_CMD    EQU 0xA0
ATAPI_IDENTIFY_CMD  EQU 0xA1
ATA_IDENTIFY_CMD        EQU 0xEC ; Command to identify drive
ATA__EXECUTE_DIAGNOSTIC EQU 0x90 ; Execute diagnostic command

;-------------------------
; Status register bits
;-------------------------
STAT_ERR            EQU 1 << 0 ; Error bit
STAT_DRQ            EQU 1 << 3 ; Data request bit
STAT_SRV            EQU 1 << 4 ; Service bit
STAT_DF             EQU 1 << 5 ; Device fault bit
STAT_RDY            EQU 1 << 6 ; Drive ready bit
STAT_BSY            EQU 1 << 7 ; Busy bit


ATA_MASTER EQU 0xA0 ; Master drive
ATA_SLAVE  EQU 0xB0 ; Slave drive

;-------------------------
; FUNCTIONS
;-------------------------

TYPE_ATAPI EQU 0x00 ; ATAPI disk/Packet device
TYPE_ATA  EQU 0x01 ; ATA disk
TYPE_NO_DISK EQU 0x02 ; No disk present
; EAX DETECT_ATAPI(al <SLAVE|MASTER>, cx <ATA_BASE|ATA_SECOND_BASE>)
; Returns:
; AL = TYPE_ATAPI. ATAPI is detected
; AL = TYPE_ATA. ATA is detected
; AL = TYPE_NO_DISK. No disk is detected
; AH = Drive type
;
; This function will detect if ATAPI is present in the system.
; ATAPI is needed for reading and writing CD-ROMs.
; Input: AL = drive (0xA0 master, 0xB0 slave)
; CX = base port (0x1F0 for primary, 0x170 for secondary)
DETECT_ATA_TYPE:
    CREATE_STACK (2*2) + (4*1)
    ; [ebp-2] = drive
    ; [ebp-4] = base port
    ; [ebp-6] = sector count results
    ; [ebp-7] = LBA low results
    ; [ebp-8] = LBA mid results
    ; [ebp-9] = LBA high results
    mov [ebp-2], al ; Save drive
    mov [ebp-4], cx ; Save base port

    ; Select drive
    mov bx, [ebp-4]
    add bx, ATA_DRIVE_HEAD
    outb bx, byte [ebp-2]
    mov ax, [ebp-4]
    add ax, ATA_COMM_REGSTAT
    inb ax
    call ATA_WAIT_FOR_BSY ; Wait for BSY to be cleared
    

    mov ax, [ebp-4]
    add ax, ATA_COMM_REGSTAT
    mov dx, ax
    mov al, ATA__EXECUTE_DIAGNOSTIC
    out dx, al ; Send command

    mov ax, [ebp-4]
    add ax, ATA_SECCOUNT
    inb ax
    mov byte [ebp-6], al ; Save sector count

    mov ax, [ebp-4]
    add ax, ATA_LBA_LO 
    inb ATA_LBA_LO 
    mov byte [ebp-7], al ; Save LBA low

    mov ax, [ebp-4]
    add ax, ATA_LBA_MID
    inb ax
    mov byte [ebp-8], al ; Save LBA mid

    mov ax, [ebp-4]
    add ax, ATA_LBA_HI
    inb ax
    mov byte [ebp-9], al ; Save LBA high

    cmp byte [ebp-6], 0x01
    jne .NO_DISK
    cmp byte [ebp-7], 0x01
    jne .NO_DISK

    cmp byte [ebp-8], 0x00
    je .TYPE_ATA
    cmp byte [ebp-9], 0x00
    je .TYPE_ATA

    cmp byte [ebp-8], 0x14
    je .TYPE_ATAPI
    cmp byte [ebp-9], 0xEB
    je .TYPE_ATAPI
.TYPE_ATAPI:
    mov ah, byte [ebp-4]
    mov al, TYPE_ATAPI
    CLEAN_STACK
    ret
.TYPE_ATA:
    mov ah, byte [ebp-4]
    mov al, TYPE_ATA
    CLEAN_STACK
    ret
.NO_DISK:
    mov ah, byte [ebp-4]
    mov al, TYPE_NO_DISK
    CLEAN_STACK
    ret


ATA_DELAY:
    inb ATA_PRIMARY_COMM_REGSTAT
    inb ATA_PRIMARY_COMM_REGSTAT
    inb ATA_PRIMARY_COMM_REGSTAT
    inb ATA_PRIMARY_COMM_REGSTAT
    ret

ATA_WAIT_FOR_BSY:
    push eax
    xor eax, eax
.loop:
    inb ATA_PRIMARY_COMM_REGSTAT
    test al, STAT_BSY
    jnz .loop ; Wait for BSY to be cleared

    pop eax
    ret

%endif ; ATA_SHARED_INC