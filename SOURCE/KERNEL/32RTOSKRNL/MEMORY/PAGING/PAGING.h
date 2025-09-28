#ifndef PAGING_H
#define PAGING_H

#include <STD/TYPEDEF.h>

#define PAGE_SIZE 0x1000
#define PAGE_ENTRIES 1024

#define PAGE_PRESENT   0x1
#define PAGE_WRITE     0x2
#define PAGE_USER      0x4

#define USER_STACK_SIZE   (8 * 1024)     // 8 KiB stack
#define USER_HEAP_SIZE    (64 * 1024)    // 64 KiB initial heap
#define USER_BINARY_BASE  0x00000000      // start of user code
#define USER_STACK_BASE   (MEM_USER_SPACE_END_MIN - USER_STACK_SIZE) // stack grows down

VOID INIT_PAGING(VOID);
VOID MAP_USER_PAGE(VOIDPTR vaddr, VOIDPTR paddr, U32 flags);

#endif // PAGING_H