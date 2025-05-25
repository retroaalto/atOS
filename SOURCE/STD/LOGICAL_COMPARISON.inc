; SOURCE/STD/LOGICAL_COMPARISON.asm - Logical comparison operations and macros
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;   This file contains macros and functions for logical comparison operations.
;
; AUTHORS
;   Antonako1
;
; REVISION HISTORY
;   2025/05/23 - Antonako1
;       Created this file with basic if-else-endif macros
;
; REMARKS
;    %include "SOURCE/STD/LOGICAL_COMPARISON.inc"
%ifndef LOGICAL_COMPARISON_INC
%define LOGICAL_COMPARISON_INC

%macro IF 1
    %rep %1
        if
    %endrep
%endmacro

%macro ELIF 1
    %rep %1
        elseif
    %endrep
%endmacro

%macro ELSE 1
    %rep %1
        else
    %endrep
%endmacro

%macro ENDIF 1
    %rep %1
        endif
    %endrep
%endmacro


%endif ; LOGICAL_COMPARISON_INC