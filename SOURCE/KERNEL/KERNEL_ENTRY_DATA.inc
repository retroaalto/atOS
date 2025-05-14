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
%define BOOLEAN db

%define _MAX_PATH 255   ; Maximum path length
%define _MAX_DRIVE 3   ; Maximum drive length
%define _MAX_DIR  255   ; Maximum directory length
%define _MAX_FNAME 255 ; Maximum file name length
%define _MAX_EXT  255   ; Maximum file extension length

%define BIT_TO_BYTE(x) (x/2)
%define BYTE_TO_BIT(x) (x*8)

drive_number: db 0
_32_bit_on: BOOLEAN 0


;%%%%%%%%%%%%%%%%%%%%%%%
; INT 15h, AX E820h
;%%%%%%%%%%%%%%%%%%%%%%%
MEM_BUF_LEN equ 256
mem_buf:
    times MEM_BUF_LEN db 0
SMAP equ 0x534D4150
SMAP_STR: db "SMAP", 4
E820_present: BOOLEAN 0

; Address Range Descriptor Structure
; 
; Offset in Bytes		Name		Description
; 	 0	    BaseAddrLow		Low 32 Bits of Base Address
; 	 4	    BaseAddrHigh	High 32 Bits of Base Address
; 	 8	    LengthLow		Low 32 Bits of Length in Bytes
; 	 12	    LengthHigh		High 32 Bits of Length in Bytes
; 	 16	    Type		Address type of  this range.
; 
; The BaseAddrLow and BaseAddrHigh together are the 64 bit BaseAddress of this range. The BaseAddress is the physical address of the start of the range being specified.
; 
; The LengthLow and LengthHigh together are the 64 bit Length of this range. The Length is the physical contiguous length in bytes of a range being specified.
; 
; The Type field describes the usage of the described address range as defined in the table below.



;%%%%%%%%%%%%%%%%%%%%%%%
; String constants
;%%%%%%%%%%%%%%%%%%%%%%%
msg_greeting_1: db "atOS-RT 16-bit BIOS kernel loaded...", 0
msg_kernel_end: db "atOS-RT 16-bit BIOS kernel ended, going into 32-bit protected mode...", 0
msg_test_1: db "MSG_TEST_1", 0
msg_mem_err1: db 'Error INT 15h, AX E820h.', 0
msg_mem_err2: db 'Error INT 15h, AX E820h. EAX != 0x534D4150', 0
msg_mem_err3: db 'Error INT 15h, AX E820h. ECX < 20', 0
msg_mem_err4: db 'Error INT 15h, AX E820h. ECX > MEM_BUF_LEN', 0
msg_hng_1: db "__HANG__", 0

;%%%%%%%%%%%%%%%%%%%%%%%
; KRNL reading
;%%%%%%%%%%%%%%%%%%%%%%%
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
name_buf: db 10

%endif ;KERNEL_ENTRY_DATA