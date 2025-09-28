#ifndef PAGEFRAME_H
#define PAGEFRAME_H

#include <STD/TYPEDEF.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif



void kernel_heap_init(void);
void user_heap_init(void);

typedef struct {
    U32 freeMemory; // in bytes
    U32 reservedMemory; // in bytes
    U32 usedMemory; // in bytes
    U32 pIndex; // page index (last search pos)
    BOOLEAN initialized; // boolean
} PAGEFRAME_INFO;

PAGEFRAME_INFO* GET_PAGEFRAME_INFO();

BOOLEAN PAGEFRAME_INIT();
U32 GET_FREE_RAM();
U32 GET_USED_RAM();
U32 GET_RESERVED_RAM();
VOIDPTR REQUEST_PAGE();
VOID RESET_PAGEINDEX();
VOID FREE_PAGE(VOIDPTR addr);
VOID FREE_PAGES(VOIDPTR addr, U32 numPages);
VOID LOCK_PAGE(VOIDPTR addr);
VOID LOCK_PAGES(VOIDPTR addr, U32 numPages);
VOID RESERVE_PAGE(VOIDPTR addr);
VOID RESERVE_PAGES(VOIDPTR addr, U32 numPages);
VOID UNRESERVE_PAGE(VOIDPTR addr);
VOID UNRESERVE_PAGES(VOIDPTR addr, U32 numPages);


typedef struct KERNEL_HEAP_BLOCK {
    U32 size;                      // total size including header
    BOOLEAN free;                   // free or used
    struct KERNEL_HEAP_BLOCK *next; // next block in heap
} KERNEL_HEAP_BLOCK;

typedef struct USER_HEAP_BLOCK {
    U32 size;                      // total size including header
    BOOLEAN free;                   // free or used
    struct USER_HEAP_BLOCK *next; // next block in heap
} USER_HEAP_BLOCK;

static inline U32 pages_from_bytes(U32 bytes) {
    return (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}

#endif // PAGEFRAME_H