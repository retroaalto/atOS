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
PORT_DRIVE_SELECT   EQU 0x1F6 ; Drive select register. Used to select the drive.
PORT_STATUS_REG     EQU 0x1F7 ; Status register. Contains status flags.

%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/DISK_DRIVER.asm"
%endif ; DISK_DRIVER_INC