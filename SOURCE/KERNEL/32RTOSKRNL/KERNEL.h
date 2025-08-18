/*+++
    Source/KERNEL/32RTOSKRNL/KERNEL.h - 32-bit Kernel Entry master header

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit kernel entry point master header for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version. Contains kernel entry point declarations and text-mode output API.

REMARKS
    To be used only by the kernel entry point implementation.
    This file should NOT be included in user applications or libraries.
---*/

#ifndef KERNEL_H
#define KERNEL_H
#include "../../STD/ATOSMINDEF.h" /* your U0, U8, U16, U32, etc. */

#define RM2LA(seg, off)  (((U32)(seg) << 4) + (U32)(off))


/* -------------------- Kernel Entry -------------------- */

/* Forward declaration of the main kernel entry function */
__attribute__((noreturn))
U0 kernel_entry_main(U0);

/* Startup function called by bootloader / linker script */
__attribute__((noreturn, section(".text")))
void _start(void);

/* -------------------- Text-mode output API -------------------- */



/* Prints a single character at the current cursor */
void put_char(U8 ch);

/* Prints a null-terminated string */
void print_string(CONST CHAR* s);

/* Prints a string with a specified length */
void print_string_len(CONST CHAR* s, U32 len);

void print_string_len_label(CONST CHAR* label, CONST CHAR* s, U32 len);

/* Prints a 32-bit unsigned integer in decimal */
void print_u32(U32 v);

/* Prints a 32-bit value in hexadecimal */
void print_hex32(U32 v);

/* Prints a newline */
static inline void print_crlf() { put_char('\n'); }

/* Prints a label and a 32-bit value in hex */
void print_label_hex(CONST CHAR* label, U32 value);

/* Prints a label and a 32-bit value in decimal */
void print_label_u32(CONST CHAR* label, U32 value);

void clear_screen(U0);

#endif /* KERNEL_H */
