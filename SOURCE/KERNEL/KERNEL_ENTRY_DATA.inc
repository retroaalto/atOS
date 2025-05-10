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

msg_greeting_1:  db "Greetings from kernel!", 0
drive_number: db 0

%endif ;KERNEL_ENTRY_DATA