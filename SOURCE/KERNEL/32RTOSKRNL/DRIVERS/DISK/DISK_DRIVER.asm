; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\VIDEO\VIDEO_DRIVER.asm - Screen driver
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Screen driver for 32RTOSKRNL.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Created this file
; 
; REMARKS
;     Only to be used by the kernel.
[BITS 32]
%ifndef DISK_DRIVER_ASM
%define DISK_DRIVER_ASM
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/DISK_DRIVER.inc"



%endif ; DISK_DRIVER_ASM