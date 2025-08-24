/*+++
    Source/KERNEL/32RTOSKRNL/CPU/GDT/GDT.h - GDT Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit GDT and IDT definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains GDT structures and constants.

REMARKS

    Include ..\INTERUPTS.h before this file.
    When compiling include GDT.c
    Only for kernel usage.
---*/

#ifndef GDT_H
#define GDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#define GDT_START MEM_GDT_BASE
#define GDT_END   MEM_GDT_END
#define GDT_SIZE  (GDT_END - GDT_START + 1)

typedef struct __attribute__((packed)) {
    U16 LINK;
    U16 LINK_RESERVED;
    U32 ESP0;
    U16 SS0;
    U16 SS0_RESERVED;
    U32 ESP1;
    U16 SS1;
    U16 SS1_RESERVED;
    U32 ESP2;
    U16 SS2;
    U16 SS2_RESERVED;
    U32 CR3;
    U32 EIP;
    U32 EFLAGS;
    U32 EAX;
    U32 ECX;
    U32 EDX;
    U32 EBX;
    U32 ESP;
    U32 EBP;
    U32 ESI;
    U32 EDI;
    U16 ES;
    U16 ES_RESERVED;
    U16 CS;
    U16 CS_RESERVED;
    U16 SS;
    U16 SS_RESERVED;
    U16 DS;
    U16 DS_RESERVED;
    U16 FS;
    U16 FS_RESERVED;
    U16 GS;
    U16 GS_RESERVED;
    U16 LDTR;
    U16 LDTR_RESERVED;
    U16 IOPB_RESERVED;
    U16 IOPB;
    U32 SSP;
} TSS;

typedef struct __attribute__((packed)) {
} GDTENTRY;

#define GDT_ENTRY(base, limit, access, flags) \
    (((base) & 0xFFFFFF) | ((limit) & 0xFFFF) << 16 | ((access) & 0xFF) << 40 | ((flags) & 0xF) << 52)

/*
GDT is limited to 32 entries.
*/
#define GDT_MAX_ENTRIES 32

#define KERNEL_DESCRIPTOR_OFFSET 0x0000
#define NULL_DESCRIPTOR GDT_ENTRY(0, 0, 0, 0)

#define KERNEL_CODE_DESCRIPTOR_OFFSET 0x0008
#define KERNEL_CODE_DESCRIPTOR GDT_ENTRY(0, 0xFFFFF, 0x9A, 0xC)

#define KERNEL_DATA_DESCRIPTOR_OFFSET 0x0010
#define KERNEL_DATA_DESCRIPTOR GDT_ENTRY(0, 0xFFFFF, 0x92, 0xC)

#define USER_SEGMENT_OFFSET 0x0018
#define USER_CODE_SEGMENT GDT_ENTRY(0, 0xFFFFF, 0xFA, 0xC)

#define USER_DATA_SEGMENT_OFFSET 0x0020
#define USER_DATA_SEGMENT GDT_ENTRY(0, 0xFFFFF, 0xF2, 0xC)

#define TASK_STATE_SEGMENT_OFFSET 0x0028
#define TASK_STATE_SEGMENT GDT_ENTRY(0, sizeof(TSS)-1, 0x89, 0x0)

BOOLEAN GDT_INIT(U0);
#endif // GDT_H
