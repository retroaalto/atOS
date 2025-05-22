; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\DISK\ATAPI\DISK_DRIVER.asm - ATAPI Disk Driver
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     ATAPI Disk Driver for 32RTOSKRNL.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Created this file with ATAPI Identify,
;         ATAPI Read Sectors and ATAPI Write Sectors functions.
; 
; REMARKS
;     Only to be used by the kernel.
[BITS 32]
%ifndef ATAPI_DISK_DRIVER_ASM
%define ATAPI_DISK_DRIVER_ASM
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATAPI/DISK_DRIVER.inc"
%include "SOURCE/STD/STACK.inc"
%include "SOURCE/STD/CPU_IO.inc"

%endif ; ATAPI_DISK_DRIVER_ASM