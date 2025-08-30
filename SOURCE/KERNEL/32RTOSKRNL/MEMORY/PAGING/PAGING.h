#ifndef PAGING_H
#define PAGING_H

#include <STD/ATOSMINDEF.h>

typedef struct __attribute__((packed)) {
    U32 present          : 1;  // Page is present in memory
    U32 rw               : 1;  // Read/write (1=writeable)
    U32 user             : 1;  // User/supervisor (1=user)
    U32 write_through    : 1;  // Page-level write-through
    U32 cache_disable    : 1;  // Disable caching
    U32 accessed         : 1;  // Has the page been accessed
    U32 dirty            : 1;  // Has the page been written to (only in PTE)
    U32 pat              : 1;  // Page Attribute Table (ignored sometimes)
    U32 global           : 1;  // Global page, not flushed by TLB
    U32 available        : 3;  // Available for OS use
    U32 frame            : 20; // Upper 20 bits of physical frame address
} PageTableEntry;

typedef struct {
    PageTableEntry entries[1024];
} __attribute__((packed)) PageDirectory;

VOID INIT_PAGING(VOID);
VOID SWITCH_PAGE(VOID);
VOID MAP_PAGE(U32 physAddr, U32 virtAddr);
VOID UNMAP_PAGE(U32 virtAddr);

#endif // PAGING_H