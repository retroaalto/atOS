; SOURCE/STD/CPU_IO.inc - CPU I/O include file
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the definitions and constants used for CPU I/O operations.
;
; AUTHORS
;     Antonako1
;
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Initial version. Contains basic I/O operations.
;
; REMARKS
;     %include "SOURCE/STD/CPU_IO.inc"

%ifndef STD_CPU_IO_INC
%define STD_CPU_IO_INC

%macro inb 1
    mov dx, %1
    in al, dx
%endmacro

%macro outb 2
    mov dx, %1
    mov al, %2
    out dx, al
%endmacro

%endif ; STD_CPU_IO_INC