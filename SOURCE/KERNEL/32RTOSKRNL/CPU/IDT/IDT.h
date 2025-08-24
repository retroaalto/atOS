/*+++
    Source/KERNEL/32RTOSKRNL/CPU/IDT/IDT.h - IDT Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit IDT definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains IDT structures and constants.

REMARKS

    Include ..\INTERUPTS.h before this file.
    When compiling include IDT.c
    Only for kernel usage.
---*/

#ifndef IDT_H
#define IDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#define IDT_START MEM_IDT_BASE
#define IDT_END   MEM_IDT_END
#define IDT_SIZE  (IDT_END - IDT_START + 1)

typedef struct __attribute__((packed)) {
    U16 OffsetLow;
    U16 Selector;
    U8  Zero;
    U8  TypeAttr;
    U16 OffsetHigh;
} IDTENTRY;

typedef struct __attribute__((packed)) {
    U16 size;
    U32 offset;
} IDTDESCRIPTOR;

void IDT_INIT(void);

#endif // IDT_H