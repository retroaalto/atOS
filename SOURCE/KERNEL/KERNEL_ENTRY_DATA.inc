; SOURCE/KERNEL/KERNEL_ENTRY_DATA.inc - Kernel entry data
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the BIOS header information and includes necessary files for the BIOS.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         - Created the file.
; 
; REMARKS
;     None
%ifndef KERNEL_ENTRY_DATA
%define KERNEL_ENTRY_DATA

%define _MAX_PATH 255   ; Maximum path length
%define _MAX_DRIVE 3   ; Maximum drive length
%define _MAX_DIR  255   ; Maximum directory length
%define _MAX_FNAME 255 ; Maximum file name length
%define _MAX_EXT  255   ; Maximum file extension length

drive_number: db 0

cur_path: db _MAX_PATH dup(0)   ; Current path
cur_drive: db _MAX_DRIVE dup(0) ; Current drive


; String constants
msg_greeting_1: db "atOS-RT 16-bit BIOS kernel loaded...", 0
msg_kernel_end: db "atOS-RT 16-bit BIOS kernel ended, going into 32-bit protected mode...", 0

%endif ;KERNEL_ENTRY_DATA