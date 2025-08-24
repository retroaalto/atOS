/*+++
    Source/KERNEL/32RTOSKRNL/CPU/LDT/LDT.h - LDT Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit LDT definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains LDT structures and constants.

REMARKS

    Include ..\INTERUPTS.h not this file.
    When compiling include LDT.c
    Only for kernel usage.
---*/
#ifndef LDT_H
#define LDT_H
#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"
#define LDT_START MEM_LDT_BASE
#define LDT_END   MEM_LDT_END
#define LDT_SIZE  (LDT_END - LDT_START + 1)

#endif // LDT_H