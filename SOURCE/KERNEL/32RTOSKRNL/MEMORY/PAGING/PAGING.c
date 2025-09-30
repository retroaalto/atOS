#include <STD/MEM.h>
#include <STD/ASM.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <MEMORY/MEMORY.h>
#include <PAGING/PAGING.h>
#include <STD/STRING.h>
#include <ERROR/ERROR.h>
#include <RTOSKRNL_INTERNAL.h>

#define PAGE_ENTRIES 1024

static ADDR *page_directory __attribute__((section(".data"))) = NULL;
static ADDR *next_free_table __attribute__((section(".data"))) = NULL;

// Load CR3 with page directory physical address
VOID load_page_directory(ADDR phys_addr) {
    ASM_VOLATILE("mov %0, %%cr3" : : "r"(phys_addr) : "memory");
}

// Enable paging by setting CR0.PG
VOID enable_paging(VOID) {
    U32 cr0;
    ASM_VOLATILE("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG
    ASM_VOLATILE("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

// Initialize identity-mapped paging for all of 32-bit memory
BOOLEAN PAGING_INIT(VOID) {
    if (page_directory != NULL) return TRUE; // Already initialized

    // Allocate page directory
    page_directory = (ADDR *)REQUEST_PAGE();
    panic_if(!page_directory, PANIC_TEXT("Failed to allocate page directory"), PANIC_OUT_OF_MEMORY);
    MEMZERO(page_directory, PAGE_SIZE);

    // Calculate total tables needed to map 4GB
    // Each table maps 4MB (1024 * 4KB)
    U32 total_tables = 1024; // 4GB / 4MB = 1024 tables

    
    
    next_free_table = (ADDR *)REQUEST_PAGES(total_tables);
    panic_if(!next_free_table, PANIC_TEXT("Failed to allocate memory for page tables"), PANIC_OUT_OF_MEMORY);
    MEMZERO(next_free_table, total_tables * PAGE_SIZE);
    
    // Fill page directory
    for (U32 pd_index = 0; pd_index < PAGE_ENTRIES; pd_index++) {
        ADDR *page_table = next_free_table;
        page_directory[pd_index] = ((ADDR)page_table & ~0xFFF) | PAGE_PRW;
        MEMZERO(page_table, PAGE_SIZE);

        // Fill page table entries
        for (U32 pt_index = 0; pt_index < PAGE_ENTRIES; pt_index++) {
            U32 addr = (pd_index << 22) | (pt_index << 12);
            page_table[pt_index] = (addr & ~0xFFF) | PAGE_PRW; // identity mapping
            
        }
        
        next_free_table = (ADDR *)((U8 *)next_free_table + PAGE_SIZE);
    }

    // Load page directory and enable paging
    load_page_directory((ADDR)page_directory);
    enable_paging();

    return TRUE;
}
