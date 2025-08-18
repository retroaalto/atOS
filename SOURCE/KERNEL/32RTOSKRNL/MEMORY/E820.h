/*+++
    SOURCE/KERNEL/32RTOSKRNL/MEMORY/E820.h - E820 Memory Map Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.
---*/
#define E820_TABLE_PHYS   0x8000u

typedef struct __attribute__((packed)) _E820_ENTRY {
    U32 BaseAddressLow;
    U32 BaseAddressHigh;
    U32 LengthLow;
    U32 LengthHigh;
    U32 Type;
} E820_ENTRY;
