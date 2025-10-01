/*
READ ME: Paging and Address Space Overview
===============================================================================
This is a simple 32-bit, single-level paging implementation with basic process isolation.
It provides ring0 paging for atOS-RT.

Kernel is linked to 0x100000 and identity-mapped there.

"User programs" aka processes are linked to 0x08048000 and
    but mapped to anywhere in physical memory

They have their own page directory and tables, created on RUN_BINARY()
They have their own heap and stack, created on RUN_BINARY(),
    but can touch any physical memory if they know the address (they need to
    subtract 0x08048000 to get physical address)
This is made in this way to allow user programs to be simple flat binaries,
    without needing relocations or ELF parsing and having kernel-level control
    over the computer.
But this means that user programs need to use syscalls to access kernel functions
    (like printing to screen, reading keyboard, running new processes, etc)
    or just compile them as part of their binary. (keeping in mind that if the source
    code uses any physical memory addresses, they need to be adjusted by 0x08048000 accordingly)
===============================================================================
*/
#include <STD/MEM.h>
#include <STD/ASM.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <MEMORY/MEMORY.h>
#include <PAGING/PAGING.h>
#include <STD/STRING.h>
#include <VIDEO/VBE.h>
#include <ERROR/ERROR.h>
#include <PROC/PROC.h>
#include <RTOSKRNL_INTERNAL.h>


static ADDR *page_directory __attribute__((section(".data"))) = NULL;
static ADDR *next_free_table __attribute__((section(".data"))) = NULL;

ADDR *get_page_directory(VOID) {
    return page_directory;
}

static void identity_map_range(U32 *pd, U32 start, U32 end, U32 flags) {
    // inclusive start, exclusive end; page-align
    U32 s = start & ~0xFFF;
    U32 e = (end + 0xFFF) & ~0xFFF;
    for (U32 addr = s; addr < e; addr += PAGE_SIZE) {
        map_page(pd, addr, addr, flags);
    }
}

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

BOOLEAN PAGING_INIT(void) {
    if (page_directory) return TRUE;

    page_directory = (ADDR *)REQUEST_PAGE();
    panic_if(!page_directory, PANIC_TEXT("Failed to allocate page directory"), PANIC_OUT_OF_MEMORY);
    MEMZERO(page_directory, PAGE_SIZE);

    // Sanity checks on layout
    panic_if((MEM_RTOSKRNL_BASE & 0xFFF) || (MEM_RTOSKRNL_END & 0xFFF),
         PANIC_TEXT("Kernel range not page-aligned"), PANIC_INVALID_ARGUMENT);

    panic_if((MEM_KERNEL_HEAP_BASE & 0xFFF) || (MEM_KERNEL_HEAP_END & 0xFFF),
            PANIC_TEXT("Heap range not page-aligned"), PANIC_INVALID_ARGUMENT);
    panic_if((MEM_KERNEL_PF_BASE & 0xFFF) || (MEM_KERNEL_PF_END & 0xFFF),
            PANIC_TEXT("Pageframe range not page-aligned"), PANIC_INVALID_ARGUMENT);

    panic_if((STACK_0_BASE & 0xFFF) || (STACK_0_END & 0xFFF),
            PANIC_TEXT("Stack range not page-aligned"), PANIC_INVALID_ARGUMENT);

    panic_if(MEM_RTOSKRNL_END > MEM_KERNEL_HEAP_BASE,
            PANIC_TEXT("Kernel overlaps heap"), PANIC_INVALID_STATE);

    panic_if(MEM_KERNEL_HEAP_END > MEM_KERNEL_PF_BASE,
            PANIC_TEXT("Heap overlaps pageframe"), PANIC_INVALID_STATE);

    panic_if(MEM_KERNEL_PF_END + PAGE_SIZE != STACK_0_BASE,
            PANIC_TEXT("Missing guard page below stack"), PANIC_INVALID_STATE);

    panic_if(STACK_0_END + PAGE_SIZE != MEM_FRAMEBUFFER_BASE,
            PANIC_TEXT("Missing guard page above stack"), PANIC_INVALID_STATE);

    // Guard page must exist between heap and stack base
    panic_if(STACK_0_BASE < MEM_KERNEL_HEAP_END + PAGE_SIZE,
            PANIC_TEXT("No space for stack guard page"), PANIC_INVALID_STATE);

    // Also ensure we can subtract one page safely
    panic_if(STACK_0_BASE < PAGE_SIZE,
            PANIC_TEXT("Stack base too low for guard page"), PANIC_INVALID_STATE);

    // Identity-map kernel and critical regions
    identity_map_range(page_directory, MEM_LOW_RESERVED_BASE, MEM_LOW_RESERVED_END, PAGE_PRW);
    identity_map_range(page_directory, MEM_E820_BASE,          MEM_E820_END,          PAGE_PRW);
    identity_map_range(page_directory, MEM_VESA_BASE,          MEM_VESA_END,          PAGE_PRW);
    VBE_MODEINFO *vmi = GET_VBE_MODE();
    if(vmi && vmi->PhysBasePtr && vmi->XResolution && vmi->YResolution && vmi->BitsPerPixel) {
        U32 fb_start = vmi->PhysBasePtr;
        U32 fb_size = vmi->YResolution * vmi->BytesPerScanLine;
        U32 fb_end = fb_start + fb_size;
        identity_map_range(page_directory, fb_start, fb_end, PAGE_PRW);
    }
    identity_map_range(page_directory, MEM_RTOSKRNL_BASE,      MEM_RTOSKRNL_END,      PAGE_PRW);
    identity_map_range(page_directory, MEM_KERNEL_HEAP_BASE,   MEM_KERNEL_HEAP_END,   PAGE_PRW);
    identity_map_range(page_directory, MEM_KERNEL_PF_BASE,     MEM_KERNEL_PF_END,     PAGE_PRW);
    identity_map_range(page_directory, STACK_0_BASE,           STACK_0_END,           PAGE_PRW);
    identity_map_range(page_directory, MEM_FRAMEBUFFER_BASE,   MEM_FRAMEBUFFER_END,   PAGE_PRW);

    load_page_directory((ADDR)page_directory);
    enable_paging();
    return TRUE;
}



void map_page(U32 *pd, U32 virt, U32 phys, U32 flags) {
    U32 pd_index = (virt >> 22) & 0x3FF;
    U32 pt_index = (virt >> 12) & 0x3FF;

    U32 pt_phys;
    if (pd[pd_index] & PAGE_PRESENT) {
        pt_phys = pd[pd_index] & ~0xFFF;
    } else {
        pt_phys = (U32)REQUEST_PAGE();
        panic_if(!pt_phys, PANIC_TEXT("Failed to allocate page table"), PANIC_OUT_OF_MEMORY);
        MEMZERO(phys_to_virt_pd(pt_phys), PAGE_SIZE);
        pd[pd_index] = (pt_phys & ~0xFFF) | PAGE_PRW;
    }

    U32 *pt = (U32 *)phys_to_virt_pd(pt_phys);
    pt[pt_index] = (phys & ~0xFFF) | (flags & 0xFFF);

    ASM_VOLATILE("invlpg (%0)" : : "r"(virt) : "memory");
}

BOOLEAN unmap_page(U32 *pd, U32 virt) {
    U32 pd_index = (virt >> 22) & 0x3FF;
    U32 pt_index = (virt >> 12) & 0x3FF;

    if (!(pd[pd_index] & PAGE_PRESENT)) {
        return FALSE; // Page table not present
    }

    U32 pt_phys = pd[pd_index] & ~0xFFF;
    U32 *pt = (U32 *)phys_to_virt_pd(pt_phys);

    if (!(pt[pt_index] & PAGE_PRESENT)) {
        return FALSE; // Page not mapped
    }

    pt[pt_index] = 0; // Unmap the page

    ASM_VOLATILE("invlpg (%0)" : : "r"(virt) : "memory");
    return TRUE;
}

ADDR KPAGE_REQUEST(VOID) {
    ADDR *page = (ADDR *)REQUEST_PAGE();
    panic_if(!page, PANIC_TEXT("PANIC: Unable to request page!"), PANIC_OUT_OF_MEMORY);
    map_page(page_directory, (U32)page, (U32)page, PAGE_PRW);
    return (ADDR)(page);
}
