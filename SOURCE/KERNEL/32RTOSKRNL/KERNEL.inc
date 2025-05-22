; SOURCE\KERNEL\32RTOSKRNL\KERNEL.inc - KRNL include file
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Include file for the KRNL (32-bit kernel entry point).
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/21 - Antonako1
;         Created this file.
; 
; REMARKS
;     None
[BITS 32]
%ifndef KERNEL_INC
%define KERNEL_INC

drive_number:       db 0
%include "SOURCE/KERNEL/32RTOSKRNL/MEMORY/MEMORY.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATAPI/DISK_DRIVER.inc"

%endif