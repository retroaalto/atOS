; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\DISK\ATAPI\DISK_DRIVER.inc - ATAPI disk driver include file
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the definitions and constants used by the ATAPI disk driver.
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
%ifndef ATAPI_DISK_DRIVER_INC
%define ATAPI_DISK_DRIVER_INC

%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATAPI/DISK_DRIVER.asm"
%endif ; DISK_DRIVER_INC