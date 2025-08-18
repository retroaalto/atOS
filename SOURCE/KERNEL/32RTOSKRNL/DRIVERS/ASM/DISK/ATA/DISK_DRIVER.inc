; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\DISK\ATA\DISK_DRIVER.inc - Disk driver include file
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the definitions and constants used by the ATA PIO disk driver.
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
%ifndef ATA_DISK_DRIVER_INC
%define ATA_DISK_DRIVER_INC

; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
; % ATA PIO Disk Driver
; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




; Drive/Head selection
BASE_MASTER_LBA     EQU 0xE0 ; Base for master and lba mode
BASE_SLAVE_LBA      EQU 0xF0 ; Base for slave and lba mode

; Command codes
ATA_READ_SECTOR_CMD     EQU 0x20 ; Command to read sectors
ATA_WRITE_SECTOR_CMD    EQU 0x30 ; Command to write sectors

%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATA_SHARED.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATA/DISK_DRIVER.asm"
%endif ; ATA_DISK_DRIVER_INC