; SOURCE\KERNEL\32RTOSKRNL\RTOSKRNL.inc - RTOSKRNL include file
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     Master include file for the 32RTOSKRNL.
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
%ifndef RTOSKRNL_INC
%define RTOSKRNL_INC
%include "SOURCE/KERNEL/32RTOSKRNL/MEMORY/MEMORY.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VIDEO_DRIVER.inc"

%endif ; RTOSKRNL_INC