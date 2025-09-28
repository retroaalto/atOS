// paging_setup.c
#include <STD/MEM.h>
#include <STD/STRING.h>
#include <STD/ASM.h>
#include <MEMORY/MEMORY.h>   // provides MEM_USER_SPACE_BASE, etc.
#include <PAGING/PAGEFRAME.h> // REQUEST_PAGE(), etc., used later
#include <PAGING/PAGING.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h> // panic()
#include <VIDEO/VBE.h>       // VBE_DRAW_STRING(), etc., used in panic
#include <STD/BINARY.h>

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE     1024
#define PAGE_ALIGN __attribute__((aligned(4096)))
#define FOUR_MB (0x400000u)
#define PAGE_SIZE (0x1000u)

/* number of PDEs required to identity-map [0 .. MEM_USER_SPACE_BASE) */
enum { KERNEL_PDE_COUNT = ((MEM_USER_SPACE_BASE + FOUR_MB - 1) / FOUR_MB) };

/* safety: ensure we don't exceed 1024 PDEs */
#if KERNEL_PDE_COUNT > PAGE_DIRECTORY_SIZE
#  error "MEM_USER_SPACE_BASE requires more PDEs than available"
#endif

/* aligned page directory + static tables used during INIT_PAGING */
static PAGE_ALIGN U32 page_directory[PAGE_DIRECTORY_SIZE];
static PAGE_ALIGN U32 page_tables[KERNEL_PDE_COUNT][PAGE_TABLE_SIZE];

/* Convenience flags */
#define PTE_PRESENT 0x1u
#define PTE_WRITE   0x2u
#define PTE_USER    0x4u

/* Identity-map all addresses from 0..(MEM_USER_SPACE_BASE-1) */
void INIT_PAGING(void) {
    /* wipe structures */
    MEMZERO(page_directory, sizeof(page_directory));
    MEMZERO(page_tables, sizeof(page_tables));

    /* For each PDE we need, point it to a static page table and fill entries */
    for (unsigned pde = 0; pde < KERNEL_PDE_COUNT; ++pde) {
        /* install page table base into PDE (present | rw) - no USER bit for kernel */
        page_directory[pde] = ((U32)page_tables[pde]) | PTE_PRESENT | PTE_WRITE;

        /* fill page table: identity map vaddr -> paddr for this 4MiB chunk */
        U32 base_v = pde * FOUR_MB;
        for (unsigned pti = 0; pti < PAGE_TABLE_SIZE; ++pti) {
            U32 vaddr = base_v + (pti * PAGE_SIZE);
            if (vaddr >= MEM_USER_SPACE_BASE) {
                page_tables[pde][pti] = 0;
            } else {
                /* identity map: virtual == physical; kernel pages are present+rw */
                page_tables[pde][pti] = (vaddr & 0xFFFFF000u) | PTE_PRESENT | PTE_WRITE;
            }
        }
    }


    // After mapping kernel + heap + stack
    VBE_MODEINFO *mode = GET_VBE_MODE();
    U32 fb_base = (U32)mode->PhysBasePtr;
    U32 fb_end  = fb_base + FRAMEBUFFER_SIZE;

    for (U32 addr = ALIGN_DOWN(fb_base, PAGE_SIZE); addr < fb_end; addr += PAGE_SIZE) {
        U32 dir_index   = (addr >> 22) & 0x3FF;
        U32 table_index = (addr >> 12) & 0x3FF;

        U32 *table;
        if (!(page_directory[dir_index] & PTE_PRESENT)) {
            table = page_tables[dir_index];      // static table pool
            MEMZERO(table, PAGE_SIZE);
            page_directory[dir_index] = ((U32)table) | PTE_PRESENT | PTE_WRITE;
        } else {
            table = (U32*)(page_directory[dir_index] & 0xFFFFF000);
        }

        table[table_index] = (addr & 0xFFFFF000) | PTE_PRESENT | PTE_WRITE;
    }



    /* Load CR3 with physical address of page_directory.
       Because these arrays are in the kernel image (identity), the pointer is a
       valid physical address at this moment. */
    asm volatile("mov %0, %%cr3" :: "r"((U32)page_directory) : "memory");
    /* Enable paging: set PG bit in CR0 */
    U32 cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u; /* PG */

    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");


    __asm__ __volatile__ (
        "mov %0, %%eax\n"
        : : "r"(mode->PhysBasePtr) : "memory"
    );

}








/* ------------------------------------------------------------------------- */
/* Helpers for per-process page directories (callable after kernel heap init) */
/* ------------------------------------------------------------------------- */

/* Create a fresh page directory for a user process:
   - allocate it from KMALLOC (kernel heap), zero it and copy kernel PDEs
   - kernel PDEs copied without PAGE_USER flag so user-mode can't access kernel */
U32 *create_process_pagedir(void) {
    U32 *pd = (U32 *)KMALLOC(PAGE_SIZE);
    if (!pd) return NULL;
    MEMZERO(pd, PAGE_SIZE);

    /* copy kernel PDEs so kernel address range is mapped in the process PD */
    for (unsigned i = 0; i < KERNEL_PDE_COUNT; ++i) {
        pd[i] = page_directory[i]; /* these have no PAGE_USER bit */
    }
    return pd;
}

/* Map a single user virtual page into a process pagedir (pagedir must be kernel-allocated)
   flags: combination of PTE_WRITE (writable) — PTE_USER implied for user pages */
void MAP_USER_PAGE_IN_PAGEDIR(U32 *pagedir, VOIDPTR vaddr, VOIDPTR paddr, U32 flags) {
    unsigned dir_index = (((U32)vaddr) >> 22) & 0x3FF;
    unsigned table_index = (((U32)vaddr) >> 12) & 0x3FF;
    U32 *ptable;

    if (!(pagedir[dir_index] & PTE_PRESENT)) {
        /* allocate page table from kernel heap (identity-mapped) */
        ptable = (U32 *)KMALLOC(PAGE_SIZE);
        if (!ptable) panic("Out of memory allocating user page table", 0);
        MEMZERO(ptable, PAGE_SIZE);

        /* store table address into PDE with USER bit (so user-mode can traverse) */
        pagedir[dir_index] = ((U32)ptable & 0xFFFFF000u) | PTE_PRESENT | PTE_WRITE | PTE_USER;
    } else {
        ptable = (U32 *)(pagedir[dir_index] & 0xFFFFF000u);
    }

    /* install PTE: physical page + flags + present + user */
    ptable[table_index] = (((U32)paddr) & 0xFFFFF000u) | (flags & 0xFFFu) | PTE_PRESENT | PTE_USER;

    /* invalidate TLB entry for that virtual address */
    asm volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}








/* Map a virtual page for user memory */
void MAP_USER_PAGE(VOIDPTR vaddr, VOIDPTR paddr, U32 flags) {
    U32 dir_index = ((U32)vaddr >> 22) & 0x3FF;
    U32 table_index = ((U32)vaddr >> 12) & 0x3FF;

    U32 *table;
    if (!(page_directory[dir_index] & PTE_PRESENT)) {
        // Allocate page table in kernel heap
        table = (U32*)KMALLOC(PAGE_SIZE);
        MEMSET(table, 0, PAGE_SIZE);
        page_directory[dir_index] = ((U32)table) | PTE_PRESENT | PTE_WRITE | PTE_USER;
    } else {
        table = (U32*)(page_directory[dir_index] & 0xFFFFF000);
    }

    table[table_index] = ((U32)paddr) | (flags & 0xFFF) | PTE_PRESENT | PTE_USER;

    // Invalidate TLB for this page
    asm volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void map_user_stack() {
    U32 stack_pages = pages_from_bytes(USER_STACK_SIZE);
    for (U32 i = 0; i < stack_pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) panic("PANIC: Out of memory allocating user stack!", PANIC_OUT_OF_MEMORY);
        
        // Map virtual page for stack
        MAP_USER_PAGE((VOIDPTR)(USER_STACK_BASE + i * PAGE_SIZE), phys,
                      PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
}

void map_user_binary(U32 binary_size) {
    U32 pages = pages_from_bytes(binary_size);
    for (U32 i = 0; i < pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) panic("PANIC: Out of memory allocating user binary!", PANIC_OUT_OF_MEMORY);
        
        // Map virtual page for binary
        MAP_USER_PAGE((VOIDPTR)(USER_BINARY_BASE + i * PAGE_SIZE), phys,
                      PAGE_PRESENT | PAGE_USER); // code usually read-only
    }
}

void map_user_heap() {
    static USER_HEAP_BLOCK *user_heap_start = (USER_HEAP_BLOCK *)0x00800000; // example base
    user_heap_start->size = USER_HEAP_SIZE;
    user_heap_start->free = TRUE;
    user_heap_start->next = NULL;

    U32 heap_pages = pages_from_bytes(USER_HEAP_SIZE);
    for (U32 i = 0; i < heap_pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) panic("PANIC: Out of memory allocating user heap!", PANIC_OUT_OF_MEMORY);
        
        MAP_USER_PAGE((VOIDPTR)((U32)user_heap_start + i * PAGE_SIZE), phys,
                      PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
}

void setup_user_process(U32 binary_size) {
    map_user_stack();
    map_user_binary(binary_size);
    map_user_heap();
}
