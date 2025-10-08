/*+++
    Source/STD/IO.h - Input/Output Definitions

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Standard input/output definitions for atOS.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version. Contains standard I/O function declarations.

REMARKS
    Use this header to include standard I/O functions in your application.
    This file should NOT be included in kernel or low-level drivers, 
        as it may introduce unwanted dependencies and conflicts, and
        may increase binary size.

Function table:
    - puts
    - putc
---*/
#ifndef IO_H
#define IO_H

#include <STD/TYPEDEF.h>
#include <DRIVERS/PS2/KEYBOARD.h> // For definitions

void putc(U8 c);
void puts(const U8 *str);

/*
Do NOT use:
void reset_keyboard(void);
void get_last_keypress(KEYPRESS *kp);
// KEYPRESS MUST be freed with free_keypress after use
KEYPRESS *get_current_keypress(void);
void free_keypress(KEYPRESS *kp);
void get_modifiers(MODIFIERS *mod);
*/
U8 keypress_to_char(U32 kcode);
#endif // IO_H
