; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\DISK\DISK_DRIVER.inc - Disk driver include file
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the definitions and constants used in the disk driver.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Initial version. Contains basic ports and register definitions.
; 
; REMARKS
;     Only to be used by the kernel and disk driver.
;     Include this file, not DISK_DRIVER.asm in the kernel.
[BITS 32]
%ifndef DISK_DRIVER_INC
%define DISK_DRIVER_INC


;-------------------------
; Ports
;-------------------------
PORT_DATA_REG       EQU 0x1F0 ; Data register for reading/writing. 16 bits wide
PORT_ERROR          EQU 0x1F1 ; Error register. Contains error codes.
PORT_SECTOR_COUNT   EQU 0x1F2 ; Number of sectors to read/write. 1 byte wide
PORT_LBA_LOW        EQU 0x1F3 ; LBA low register. Contains the lower 8 bits of the LBA address.
PORT_LBA_MID        EQU 0x1F4 ; LBA mid register. Contains the middle 8 bits of the LBA address.
PORT_LBA_HIGH       EQU 0x1F5 ; LBA high register. Contains the upper 8 bits of the LBA address.
PORT_DRIVE_SELECT   EQU 0x1F6 ; Drive select register. Used to select the drive. Upper 4 bits are used for the drive number.
PORT_STATUS_REG     EQU 0x1F7 ; Status (read) and command (write) register. Contains status codes and command codes.

; Drive/Head selection
BASE_MASTER_LBA     EQU 0xE0 ; Base for master and lba mode
BASE_SLAVE_LBA      EQU 0xF0 ; Base for slave and lba mode

; Command codes
READ_SECTOR_CMD     EQU 0x20 ; Command to read sectors
WRITE_SECTOR_CMD    EQU 0x30 ; Command to write sectors

; 1: 1 bit: Master = 0 or Slave = 1
; 2: 8bit: LBA_LOW
; 3: 8bit: LBA_MID
; 4: 8bit: LBA_HIGH
%macro LBA_DRIVE_SET 4
    mov dx, PORT_LBA_LOW
    mov al, %2
    out dx, al
    mov dx, PORT_LBA_MID
    mov al, %3
    out dx, al
    mov dx, PORT_LBA_HIGH
    mov al, %4
    out dx, al
    mov dx, PORT_DRIVE_SELECT
%endmacro ; LBA_DRIVE_SET
    

%macro PUSH_READ_SECTOR_PARAMS 3
    ; %1 = LBA (immediate or register)
    ; %2 = SECTOR_COUNT (immediate or register)
    ; %3 = DRIVE_HEAD (immediate or register)

    push word %3       ; DRIVE_HEAD (word)
    push byte %2       ; SECTOR_COUNT (byte — will be pushed as 4-byte word unless in 16-bit mode)
    push dword %1      ; LBA (dword)
%endmacro

; For ISO9660
%define SECTOR_SIZE             2048        ; 2KB per sector
%define VOL_DESC_START          16          ; Volume descriptor starts at sector 16
PVD_OFFSET equ              VOL_DESC_START*SECTOR_SIZE/SECTOR_SIZE



%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/DISK_DRIVER.asm"
%endif ; DISK_DRIVER_INC