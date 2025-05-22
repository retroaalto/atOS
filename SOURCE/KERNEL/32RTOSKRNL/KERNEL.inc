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

%include "SOURCE/KERNEL/32RTOSKRNL/MEMORY/MEMORY.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/ATAPI/DISK_DRIVER.inc"


msg_1           db "Kernel is running!", 0
msg_2           db "Kernel shutting down!", 0
msg_ata         db "ATA found!", 0
msg_atapi       db "ATAPI found!", 0
msg_err         db "Error reading kernel!", 0
ROW             dq 0
MASTER_SLAVE    db 0
drive_number    db 0

%endif