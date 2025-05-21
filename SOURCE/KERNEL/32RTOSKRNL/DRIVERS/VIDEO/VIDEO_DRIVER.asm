; SOURCE\KERNEL\32RTOSKRNL\DRIVERS\VIDEO\VIDEO_DRIVER.asm - Screen driver
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the screen driver for 32RTOS. It is responsible for
;     initializing the screen, setting the resolution, and handling screen
;     operations such as drawing pixels, lines, and rectangles. 
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
%ifndef SCREEN_DRIVER_ASM
%define SCREEN_DRIVER_ASM
%include "SOURCE/KERNEL/32RTOSKRNL/KERNEL.inc"
%include "SOURCE/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VIDEO_DRIVER.inc"

%endif 