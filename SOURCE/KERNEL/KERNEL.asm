; SOURCE/KERNEL/KERNEL.asm - 32-bit kernel entry point
;      
; Licensed under the MIT License. See LICENSE file in the project root for full license information.
;
; DESCRIPTION
;     32-bit kernel entry point
; 
; AUTHORS
;     Antonako1
; 
; REVISION HISTORY
;     2025/05/10 - Antonako1
;         Created this file
; 
; REMARKS
;     Additional remarks, if any.

[BITS 32]
[ORG 0x8000:0x0000]

global _start
_start:
    xor eax, eax
    mov esp, 0x100000

    mov ebx,0xb8000    ; The video address
    mov al,'!'         ; The character to be print
    mov ah,0x0F        ; The color: white(F) on black(0)
    mov [ebx],ax        ; Put the character into the video memory


HANG:
    jmp $