#ifndef PAGING_H
#define PAGING_H

#include <STD/TYPEDEF.h>
#include <MEMORY/MEMORY.h>

#define PAGE_PRESENT    0x1
#define PAGE_READ_WRITE 0x2
#define PAGE_PRW        (PAGE_PRESENT | PAGE_READ_WRITE)
#define PDE_INDEX(addr)      ((addr) >> 22)
#define KERNEL_PDE_START     (PDE_INDEX(MEM_RTOSKRNL_BASE))
#define KERNEL_PDE_END       (PDE_INDEX(MEM_RTOSKRNL_END - 1))
#define PAGE_ENTRIES 1024

BOOLEAN PAGING_INIT(VOID);
ADDR *get_page_directory(VOID);
void map_page(U32 *pd, U32 virt, U32 phys, U32 flags);

#endif // PAGING_H