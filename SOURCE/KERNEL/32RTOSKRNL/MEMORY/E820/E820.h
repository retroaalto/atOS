/*+++
    Source/KERNEL/32RTOSKRNL/MEMORY/E820.h - E820 Memory Map Definitions

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit E820 memory map definitions for atOS.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/19 - Antonako1
        Initial version. Contains E820 entry structure and constants.

REMARKS
    Use this header to include E820 memory map definitions in your application.

    Some functions, definitions and macros are only available for kernel mode.
    

    When compiling include E820.c
---*/
#ifndef E820_H
#define E820_H
#include <STD/ATOSMINDEF.h>
#include <MEMORY/MEMORY.h>

#define E820_TABLE_PHYS   MEM_E820_BASE
#define E820_TABLE_END    MEM_E820_END
#define E820_TABLE_SIZE   (E820_TABLE_END - E820_TABLE_PHYS + 1)u

typedef struct __attribute__((packed)) {
    U32 BaseAddressLow;
    U32 BaseAddressHigh;
    U32 LengthLow;
    U32 LengthHigh;
    U32 Type;
} E820_ENTRY;

typedef struct {
    U32 HeapStartAddress;
    U32 HeapEndAddress;
    U32 TotalHeap;
    U32 FreeHeap;
    U32 AtTableIndex;
} E820Info;




/*+++
Only for kernel mode.
You can use these in your application if you know what you're doing,
  but be aware of potential issues, such as memory corruption and stability.
---*/
#if __RTOS__
E820Info *GET_E820_INFO(VOID);
BOOLEAN E820_INIT(VOID);

#endif // __RTOS__
#endif // E820_H