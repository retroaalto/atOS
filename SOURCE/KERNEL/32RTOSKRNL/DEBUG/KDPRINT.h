/*+++
    SOURCE/KERNEL/32RTOSKRNL/DEBUG/KDPRINT.h - Kernel Debug Print Interface

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Simple serial debug print helpers for emitting kernel logs to the
    QEMU console.

AUTHORS
    ChatGPT Codex Agent

REVISION HISTORY
    2025/09/07 - ChatGPT Codex Agent
        Initial version.
REMARKS
    When compiling include KDPRINT.c
---*/
#ifndef ATOS_KDPRINT_H
#define ATOS_KDPRINT_H

#include "../../../../STD/ATOSMINDEF.h"

U0 kdprint(const char* message);
U0 kdprint_char(CHAR ch);
U0 kdprint_hex(U8 value);
U0 kdprint_init(U0);

#endif // ATOS_KDPRINT_H
