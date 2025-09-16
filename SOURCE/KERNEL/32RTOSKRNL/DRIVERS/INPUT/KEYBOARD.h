/*+++
    SOURCE/KERNEL/32RTOSKRNL/DRIVERS/INPUT/KEYBOARD.h - PS/2 Keyboard Driver Interface

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Minimal PS/2 keyboard driver APIs for atOS. Provides interrupt-driven
    scancode handling and a small character FIFO for consumers.

AUTHORS
    ChatGPT Codex Agent

REVISION HISTORY
    2025/09/07 - ChatGPT Codex Agent
        Initial version.
REMARKS
    When compiling include KEYBOARD.c
---*/
#ifndef ATOS_KEYBOARD_H
#define ATOS_KEYBOARD_H

#include "../../../../STD/ATOSMINDEF.h"

#define KBD_BUFFER_SIZE 64u

U0 KBD_INIT(U0);
BOOL KBD_HAS_KEY(U0);
CHAR KBD_PEEK_KEY(U0);
CHAR KBD_READ_KEY(U0);
U0 KBD_POLL(U0);

#endif // ATOS_KEYBOARD_H
