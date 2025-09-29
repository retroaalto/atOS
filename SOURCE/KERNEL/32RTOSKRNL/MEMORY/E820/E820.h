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
#include <STD/TYPEDEF.h>
#include <MEMORY/MEMORY.h>

#define E820_TABLE_PHYS   MEM_E820_BASE
#define E820_TABLE_END    MEM_E820_END
#define E820_TABLE_SIZE   (E820_TABLE_END - E820_TABLE_PHYS + 1)

#define TYPE_E820_RAM 0x01
#define TYPE_E820_RESERVED 0x02
#define TYPE_E820_ACPI_RECLAIMABLE 0x03
#define TYPE_E820_ACPI_NVS 0x04

#define E820_MAX_ENTRIES 32
static inline BOOLEAN fits_in_32(U32 hi) { return hi == 0; }

typedef struct __attribute__((packed)) {
    U32 BaseAddressLow;
    U32 BaseAddressHigh;
    U32 LengthLow;
    U32 LengthHigh;
    U32 Type;
} E820_ENTRY;

typedef struct {
    E820_ENTRY RawEntries[E820_MAX_ENTRIES]; // These are guaranteed to be RAM entries
    U32 RawEntryCount; // Number of valid entries in RawEntries
} E820Info;




/*+++
Only for kernel mode.
You can use these in your application if you know what you're doing,
  but be aware of potential issues, such as memory corruption and stability.
---*/
#if __RTOS__
E820_ENTRY *GET_E820_ENTRIES(VOID); // Returns pointer to internal array of E820 entries
E820_ENTRY *GET_E820_ENTRY(U32 index); // Returns pointer to specific E820 entry by index, or NULL if out of bounds
E820Info *GET_E820_INFO(VOID); // Returns pointer to E820Info structure
BOOLEAN E820_INIT(VOID); // Initializes the E820 entries, returns TRUE on success, FALSE on failure

#endif // __RTOS__
#endif // E820_H