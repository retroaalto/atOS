; SOURCE\STD\STACK.inc - Stack include file
;
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains definitions, constants and macros for the stack.
;
; Authors
;     Antonako1
;
; REVISION HISTORY
;     2025/05/22 - Antonako1
;         Initial version. Contains basic stack definitions and macros.
;
; REMARKS
;     %include "SOURCE/STD/STACK.inc"
%ifndef STD_STACK_INC
%define STD_STACK_INC

; Creates a stack with the given size.
%macro CREATE_STACK 1
    push ebp
    mov ebp, esp
    sub esp, %1
%endmacro

; Cleans up the stack.
%macro CLEAN_STACK 0
    mov esp, ebp
    pop ebp
%endmacro

%endif ; STD_STACK_INC