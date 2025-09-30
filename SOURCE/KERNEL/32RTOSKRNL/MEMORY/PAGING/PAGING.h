#ifndef PAGING_H
#define PAGING_H

#include <STD/TYPEDEF.h>

#define PAGE_PRESENT    0x1
#define PAGE_READ_WRITE 0x2
#define PAGE_PRW        (PAGE_PRESENT | PAGE_READ_WRITE)
#define KERNEL_PDE_START 768
#define PAGE_ENTRIES 1024

BOOLEAN PAGING_INIT(VOID);
ADDR *get_page_directory(VOID);
void map_page(U32 *pd, U32 virt, U32 phys, U32 flags);

#endif // PAGING_H