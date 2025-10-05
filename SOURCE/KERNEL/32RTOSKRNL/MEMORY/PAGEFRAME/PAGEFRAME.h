/*+++
    SOURCE/KERNEL/32RTOSKRNL/MEMORY/PAGEFRAME/PAGEFRAME.h - Page Frame Management

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Page frame management for physical memory allocation.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/10/03 - Antonako1
        Added this header.

REMARKS
    Do NOT call any of these functions before PAGEFRAME_INIT()!
    Do NOT call any of these functions if you don't know what you're doing!

    Do NOT use any physical memory addresses directly in the kernel or drivers,
    always allocate/free through the page frame manager. This ensures that
    the memory is tracked properly and avoids accidental overlaps.

    Do NOT use physical addresses in user programs, as they will not work
    once paging is enabled. User programs should use virtual addresses only.
---*/
#ifndef PAGEFRAME_H
#define PAGEFRAME_H

#include <STD/TYPEDEF.h>
#include <MEMORY/BYTEMAP/BYTEMAP.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

typedef struct {
    U32 freeMemory; // in bytes
    U32 reservedMemory; // in bytes
    U32 usedMemory; // in bytes
    U32 pIndex; // page index (last search pos)
    BOOLEAN initialized; // boolean
    U32 size; // total pages
} PAGEFRAME_INFO;

PAGEFRAME_INFO* GET_PAGEFRAME_INFO();
// BYTEMAP* GET_PAGEFRAME_BYTEMAP();
BOOLEAN PAGEFRAME_INIT();


BOOLEAN is_page_aligned(U32 addr);

U32 GET_FREE_RAM();
U32 GET_USED_RAM();
U32 GET_RESERVED_RAM();

// Resets the page index to 0, so the next search starts from the beginning
VOID RESET_PAGEINDEX();

ADDR KREQUEST_PAGES(U32 numPages);
ADDR KREQUEST_PAGE();
VOID KFREE_PAGE(ADDR addr);
VOID KFREE_PAGES(ADDR addr, U32 numPages);
VOID KLOCK_PAGE(ADDR addr);
VOID KLOCK_PAGES(ADDR addr, U32 numPages);
VOID KUNLOCK_PAGE(ADDR addr);
VOID KUNLOCK_PAGES(ADDR addr, U32 numPages);
VOID KRESERVE_PAGE(ADDR addr);
VOID KRESERVE_PAGES(ADDR addr, U32 numPages);
VOID KUNRESERVE_PAGE(ADDR addr);
VOID KUNRESERVE_PAGES(ADDR addr, U32 numPages);

ADDR KREQUEST_USER_PAGES(U32 numPages);
ADDR KREQUEST_USER_PAGE();
VOID KFREE_USER_PAGE(ADDR addr);
VOID KFREE_USER_PAGES(ADDR addr, U32 numPages);

/* helpers */
static inline U32 pages_from_bytes(U32 bytes) {
    U32 pages = bytes + PAGE_SIZE - 1;
    if(pages == 0) return 0; // avoid division by zero
    return pages / PAGE_SIZE;
}
static inline U32 page_align_down(U32 x) { return x & ~0xFFF; }
static inline U32 page_align_up(U32 x)   { return (x + 0xFFF) & ~0xFFF; }




#endif // PAGEFRAME_H