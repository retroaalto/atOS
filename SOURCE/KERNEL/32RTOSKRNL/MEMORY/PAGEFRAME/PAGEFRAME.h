#ifndef PAGEFRAME_H
#define PAGEFRAME_H

#include <STD/TYPEDEF.h>
#include <STD/BITMAP.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

typedef struct {
    U32 freeMemory; // in bytes
    U32 reservedMemory; // in bytes
    U32 usedMemory; // in bytes
    U32 pIndex; // page index (last search pos)
    BOOLEAN initialized; // boolean
} PAGEFRAME_INFO;

PAGEFRAME_INFO* GET_PAGEFRAME_INFO();
BITMAP* GET_PAGEFRAME_BITMAP();
BOOLEAN PAGEFRAME_INIT();


U32 GET_FREE_RAM();
U32 GET_USED_RAM();
U32 GET_RESERVED_RAM();

// These both return the starting address of the allocated page(s), 0 on failure

/// @brief Requests a block of contiguous pages.
/// @param numPages Number of pages to allocate.
/// @return Starting address of the allocated block, or 0 if allocation failed.
ADDR REQUEST_PAGES(U32 numPages);

/// @brief Requests a block of a single page.
/// @return Starting address of the allocated page, or 0 if allocation failed.
ADDR REQUEST_PAGE();

// Resets the page index to 0, so the next search starts from the beginning
VOID RESET_PAGEINDEX();

// Freeing, locking, unlocking, reserving and unreserving pages
VOID FREE_PAGE(ADDR addr);
VOID FREE_PAGES(ADDR addr, U32 numPages);
VOID LOCK_PAGE(ADDR addr);
VOID LOCK_PAGES(ADDR addr, U32 numPages);
VOID UNLOCK_PAGE(ADDR addr);
VOID UNLOCK_PAGES(ADDR addr, U32 numPages);
VOID RESERVE_PAGE(ADDR addr);
VOID RESERVE_PAGES(ADDR addr, U32 numPages);
VOID UNRESERVE_PAGE(ADDR addr);
VOID UNRESERVE_PAGES(ADDR addr, U32 numPages);



/* helpers */
static inline U32 bytes_to_pages(U32 bytes) {
    return (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}
static inline U32 pages_from_bytes(U32 bytes) {
    return (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}
void reserve_range(U32 start, U32 end);
void unreserve_range(U32 start, U32 end);

#endif // PAGEFRAME_H