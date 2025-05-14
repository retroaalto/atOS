; SOURCE/KERNEL/KERNEL_ENTRY_DATA.inc - Kernel entry data
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Contains constants and global variables used by KERNEL_ENTRY.asm
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
msg_test_1: db "MSG_TEST_1", 0

krnl_str: db "KRNL.BIN;1", 10
KRNL_STR_LEN equ 10
KERNEL_LOAD_SEGMENT equ 0x000000
KERNEL_LOAD_OFFSET  equ 0x100000

%define BUFFER_SEGMENT 0x0000
%define BUFFER_OFFSET  0x1000


%define SECTOR_SIZE             2048        ; 2KB per sector
%define VOL_DESC_START          16          ; Volume descriptor starts at sector 16
%define PVD_OFFSET              VOL_DESC_START*SECTOR_SIZE/SECTOR_SIZE

extentLocationLE_LBA: dd 0
extentLengthLE: dd 0
offset_var: dd 0
buffer_var: dd 0
name_buf: db 10                 ; Holds 

%endif ;KERNEL_ENTRY_DATA