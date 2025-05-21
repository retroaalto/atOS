; SOURCE\KERNEL\32RTOSKRNL\SYSCALL\SYSCALL_MIN.inc - Minimal syscall interface
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     This file contains the minimal syscall interface for 32RTOS.
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/21 - Antonako1
;         Created this file.
; 
; REMARKS
;     Include this file in your assembly code to use the minimal syscall interface.
%ifndef SYSCALL_MIN_INC
%define SYSCALL_MIN_INC

SYSCALL_PRINTCH     EQU     0x0001 ; Print character syscall

SYSCALL_MAXNUM      EQU     0xFFFF ; Maximum syscall number

; Usage:
; Push arguments onto the stack, if needed
; push arg1
; MAKE_SYSCALL <syscall_number>
; Results in EAX
;
; Example:
; push 'A' ; Push character to print
; MAKE_SYSCALL SYSCALL_PRINTCH ; Call syscall to print character
; cmp eax, 0 ; Check for success
%macro MAKE_SYSCALL 1
    %if %1 > SYSCALL_MAXNUM
        %error "Syscall number exceeds maximum limit"
    %endif
    mov eax, %1
    syscall
%endmacro

%endif ; SYSCALL_MIN_INC