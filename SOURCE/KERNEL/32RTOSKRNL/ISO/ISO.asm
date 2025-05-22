; SOURCE/KERNEL/32RTOSKRNL/ISO/ISO.asm - ISO file handling
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;        ISO file handling for 32RTOSKRNL.
;        This file will read the ISO file into memory and jump to it.
;
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/21 - Antonako1
;         Created this file
[BITS 32]
%ifndef ISO_ASM
%define ISO_ASM
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/DISK/DISK_DRIVER.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/MEMORY/MEMORY.inc"


%endif ; ISO_ASM