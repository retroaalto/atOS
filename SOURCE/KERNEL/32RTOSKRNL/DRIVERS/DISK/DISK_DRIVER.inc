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

; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
; % ATA PIO Disk Driver
; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

;-------------------------
; Ports
;-------------------------
ATA_PRIMARY_DATA    EQU 0x1F0 ; Primary ATA port
ATA_PRIMARY_ERR      EQU 0x1F1 ; Error register
ATA_PRIMARY_SECCOUNT        EQU 0x1F2  ; Sector count register
ATA_PRIMARY_LBA_LO    EQU 0x1F3 ; LBA low register
ATA_PRIMARY_LBA_MID    EQU 0x1F4 ; LBA mid register
ATA_PRIMARY_LBA_HI   EQU 0x1F5 ; LBA high register
ATA_PRIMARY_DRIVE_HEAD EQU 0x1F6 ; Drive/Head register
ATA_PRIMARY_COMM_REGSTAT   EQU 0x1F7 ; Status register
ATA_PRIMARY_ALTSTAT_DCR  EQU 0x3F6 ; Alternate status register

STAT_ERR            EQU 1 << 0 ; Error bit
STAT_DRQ            EQU 1 << 3 ; Data request bit
STAT_SRV            EQU 1 << 4 ; Service bit
STAT_DF             EQU 1 << 5 ; Device fault bit
STAT_RDY            EQU 1 << 6 ; Drive ready bit
STAT_BUSY           EQU 1 << 7 ; Busy bit

; Drive/Head selection
BASE_MASTER_LBA     EQU 0xE0 ; Base for master and lba mode
BASE_SLAVE_LBA      EQU 0xF0 ; Base for slave and lba mode

; Command codes
READ_SECTOR_CMD     EQU 0x20 ; Command to read sectors
WRITE_SECTOR_CMD    EQU 0x30 ; Command to write sectors


; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
; % SATA Disk Driver
; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/DISK_DRIVER.asm"
%endif ; DISK_DRIVER_INC