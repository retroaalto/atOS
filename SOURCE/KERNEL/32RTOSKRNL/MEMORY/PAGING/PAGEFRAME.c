#include <MEMORY/PAGING/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <MEMORY/E820/E820.h>
#include <MEMORY/MEMORY.h>
#include <STD/MEM.h>
#include <STD/BINARY.h>
#include <STD/ASM.h>
#include <STD/BITMAP.h>
#include <VIDEO/VBE.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <STD/MATH.h>

#include <MEMORY/KMALLOC/KMALLOC.h>
#include <MEMORY/UMALLOC/UMALLOC.h>


static KERNEL_HEAP_BLOCK *heap_start = NULL;
static USER_HEAP_BLOCK *user_heap_start = NULL;

static inline U32 align8(U32 size) {
    return (size + 7) & ~7U;
}

#define BYTE_TO_PAGE(x) ((x) / PAGE_SIZE)

static BITMAP pageFrameBitmap;
static PAGEFRAME_INFO pageFrameInfo;

PAGEFRAME_INFO* GET_PAGEFRAME_INFO() {
    return &pageFrameInfo;
}


/* PUBLIC API --------------------------------------------------------------*/

VOID RESET_PAGEINDEX() { pageFrameInfo.pIndex = 0; }

VOID FREE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    // If page already free, nothing to do.
    if(!BITMAP_GET(&pageFrameBitmap, index)) return;
    // Clear the bit
    if(!BITMAP_SET(&pageFrameBitmap, index, FALSE)) return;

    // Adjust accounting: this page was previously "used", so move it to free.
    // Guard against underflow.
    if (pageFrameInfo.usedMemory >= PAGE_SIZE) pageFrameInfo.usedMemory -= PAGE_SIZE;
    pageFrameInfo.freeMemory += PAGE_SIZE;
    if(pageFrameInfo.pIndex > index) {
        pageFrameInfo.pIndex = index;
    }
}

VOID FREE_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        FREE_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

VOID LOCK_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    // if already marked (reserved or used) do nothing
    if(BITMAP_GET(&pageFrameBitmap, index)) return;
    // set bit
    if(!BITMAP_SET(&pageFrameBitmap, index, TRUE)) return;
    // Update accounting: this transitions from free -> used
    if (pageFrameInfo.freeMemory >= PAGE_SIZE) pageFrameInfo.freeMemory -= PAGE_SIZE;
    pageFrameInfo.usedMemory += PAGE_SIZE;
}

VOID LOCK_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        LOCK_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

VOID RESERVE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    // if already marked (reserved or used) do nothing
    if(BITMAP_GET(&pageFrameBitmap, index)) return;
    if(!BITMAP_SET(&pageFrameBitmap, index, TRUE)) return;
    // Update accounting: this transitions from free -> reserved
    if (pageFrameInfo.freeMemory >= PAGE_SIZE) pageFrameInfo.freeMemory -= PAGE_SIZE;
    pageFrameInfo.reservedMemory += PAGE_SIZE;
}

VOID RESERVE_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        RESERVE_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

VOID UNRESERVE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    // If already free, nothing to do
    if(!BITMAP_GET(&pageFrameBitmap, index)) return;
    // Clear bit
    if(!BITMAP_SET(&pageFrameBitmap, index, FALSE)) return;
    // Update accounting: reserved -> free
    if (pageFrameInfo.reservedMemory >= PAGE_SIZE) pageFrameInfo.reservedMemory -= PAGE_SIZE;
    pageFrameInfo.freeMemory += PAGE_SIZE;
    if(pageFrameInfo.pIndex > index) {
        pageFrameInfo.pIndex = index;
    }
}

VOID UNRESERVE_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        UNRESERVE_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

/* Read E820, build bitmap and initialize counters */
BOOLEAN READ_E820_MEMORYMAP(VOID) {
    E820Info *info = GET_E820_INFO();
    if (!info) return FALSE;

    // compute total RAM size and find largest suitable RAM entry
    U32 total_ram_bytes = 0;
    U32 largest_idx = (U32)-1;
    U32 largest_size = 0;

    for (U32 i = 0; i < info->RawEntryCount; i++) {
        E820_ENTRY *e = &info->RawEntries[i];
        if (e->Type != TYPE_E820_RAM) continue;
        if (!fits_in_32(e->BaseAddressHigh) || !fits_in_32(e->LengthHigh)) continue;

        U32 base = (U32)e->BaseAddressLow;
        U32 len  = (U32)e->LengthLow;
        total_ram_bytes += len;
        if (len > largest_size) {
            largest_size = len;
            largest_idx = i;
        }
    }

    if (largest_idx == (U32)-1) return FALSE; // no usable RAM entry found

    // total pages & bitmap size (bytes)
    U32 total_pages = (total_ram_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    U32 bitmap_bytes = (total_pages + 7) / 8; // ceil(pages / 8)

    // choose placement: place bitmap at the END (high addresses) of the largest RAM entry
    // so we don't risk overlapping kernel in low memory.
    E820_ENTRY *largest = &info->RawEntries[largest_idx];
    U32 largest_base = (U32)largest->BaseAddressLow;
    U32 largest_len  = (U32)largest->LengthLow;

    // Align placement to page: we put the bitmap at top of region, page-aligned down.
    U32 place_end = ALIGN_DOWN(largest_base + largest_len, PAGE_SIZE);
    if (place_end < bitmap_bytes) return FALSE; // shouldn't happen
    U32 place_start = place_end - ALIGN_UP(bitmap_bytes, PAGE_SIZE);
    VOIDPTR bitmap_place = (VOIDPTR)place_start;

    // Reserve the pages that will hold the bitmap immediately to protect them
    U32 bitmap_pages = ALIGN_UP(bitmap_bytes, PAGE_SIZE) / PAGE_SIZE;
    // mark them reserved in bitmap data structure only after we create the bitmap object
    

    VBE_MODEINFO *mode = GET_VBE_MODE();
    
    // Create the bitmap structure pointing at the chosen area.
    BITMAP_CREATE(bitmap_bytes, bitmap_place, &pageFrameBitmap);
    U32 bmp_addr = (U32)pageFrameBitmap.data;
    U32 bmp_len  = bitmap_pages * PAGE_SIZE;

    // check overlap with kernel, framebuffer, VBE structures etc.
    if (range_overlap(bmp_addr, bmp_len, MEM_RTOSKRNL_BASE, MEM_RTOSKRNL_END - MEM_RTOSKRNL_BASE)) {
        panic("PANIC: bitmap overlaps kernel!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, MEM_FRAMEBUFFER_BASE, FRAMEBUFFER_SIZE)) {
        panic("PANIC: bitmap overlaps framebuffer!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, mode->PhysBasePtr, FRAMEBUFFER_SIZE)) {
        panic("PANIC: bitmap overlaps VBE mode!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, MEM_BIOS_BASE, MEM_BIOS_END - MEM_BIOS_BASE)) {
        panic("PANIC: bitmap overlaps BIOS area!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, STACK_0_BASE, STACK_0_END - STACK_0_BASE)) {
        panic("PANIC: bitmap overlaps stack!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, MEM_E820_BASE, MEM_E820_END - MEM_E820_BASE)) {
        panic("PANIC: bitmap overlaps E820 area!", PANIC_PAGE_FAULT_IN_KERNEL);
    }
    if (range_overlap(bmp_addr, bmp_len, MEM_VESA_BASE, MEM_VESA_END - MEM_VESA_BASE)) {
        panic("PANIC: bitmap overlaps VESA area!", PANIC_PAGE_FAULT_IN_KERNEL);
    }

    // Immediately reserve pages that contain the bitmap so subsequent bitmap ops cannot clobber kernel
    RESERVE_PAGES((VOIDPTR)pageFrameBitmap.data, bitmap_pages);

    // Initialize all bits to 1 (reserved) so we start from safe state,
    // then unreserve actual RAM areas below.
    // Use direct memset on the bitmap memory to speed up init and avoid per-bit writes.
    // Note: BITMAP.data is assumed writable and pageFrameBitmap.size >= bitmap_bytes
    MEMSET(pageFrameBitmap.data, 0xFF, bitmap_bytes);

    // Initialize accounting: everything reserved initially; we'll unreserve actual RAM ranges.
    pageFrameInfo.freeMemory = 0;
    pageFrameInfo.usedMemory = 0;
    pageFrameInfo.reservedMemory = total_pages * PAGE_SIZE;
    pageFrameInfo.pIndex = 0;

    // Unreserve actual usable RAM ranges (TYPE_E820_RAM)
    for (U32 i = 0; i < info->RawEntryCount; i++) {
        E820_ENTRY *e = &info->RawEntries[i];
        if (e->Type != TYPE_E820_RAM) continue;
        if (!fits_in_32(e->BaseAddressHigh) || !fits_in_32(e->LengthHigh)) continue;
        U32 base = ALIGN_UP((U32)e->BaseAddressLow, PAGE_SIZE);
        U32 len  = ((U32)e->LengthLow) - (base - (U32)e->BaseAddressLow); // adjust if aligned
        if (len == 0) continue;
        U32 start_page = base / PAGE_SIZE;
        U32 npages = len / PAGE_SIZE;
        for (U32 p = 0; p < npages; p++) {
            U32 idx = start_page + p;
            // clear bit if currently set
            U8 byte_idx = idx / 8;
            U8 bit_mask = (1 << (idx % 8));
            // If the bit is set (reserved), clear it and update counters
            if (pageFrameBitmap.data[byte_idx] & bit_mask) {
                pageFrameBitmap.data[byte_idx] &= ~bit_mask;
                if (pageFrameInfo.reservedMemory >= PAGE_SIZE) pageFrameInfo.reservedMemory -= PAGE_SIZE;
                pageFrameInfo.freeMemory += PAGE_SIZE;
            }
        }
    }

    // Now we have correct free/reserved accounting for RAM from E820.
    // Reserve/lock other critical ranges (these will update accounting properly via RESERVE_PAGE/LOCK_PAGE)
    RESERVE_PAGES(0, 0x100); // low BIOS area

    // Reserve E820 structure region
    RESERVE_PAGES(MEM_E820_BASE, ALIGN_UP(MEM_E820_END - MEM_E820_BASE, PAGE_SIZE) / PAGE_SIZE);

    // VESA/VBE areas (if valid)
    RESERVE_PAGES(MEM_VESA_BASE, ALIGN_UP(MEM_VESA_END - MEM_VESA_BASE, PAGE_SIZE) / PAGE_SIZE);
    if (mode && mode->PhysBasePtr) {
        RESERVE_PAGES((VOIDPTR)mode->PhysBasePtr, ALIGN_UP(FRAMEBUFFER_SIZE, PAGE_SIZE) / PAGE_SIZE);
    }
    RESERVE_PAGES(MEM_FRAMEBUFFER_BASE, ALIGN_UP(FRAMEBUFFER_SIZE, PAGE_SIZE) / PAGE_SIZE);
    RESERVE_PAGES(MEM_BIOS_BASE, ALIGN_UP(MEM_BIOS_END - MEM_BIOS_BASE, PAGE_SIZE) / PAGE_SIZE);

    // Lock RTOS kernel code & data (these should move from free -> used)
    LOCK_PAGES(MEM_RTOSKRNL_BASE, ALIGN_UP(MEM_RTOSKRNL_END - MEM_RTOSKRNL_BASE, PAGE_SIZE) / PAGE_SIZE);

    // Do NOT lock whole heap here — leave it available for allocations (previous bug)
    // Lock stack
    LOCK_PAGES((VOIDPTR)STACK_0_BASE, ALIGN_UP(STACK_0_END - STACK_0_BASE, PAGE_SIZE) / PAGE_SIZE);

    // Bitmap pages are already reserved above. Done.
    return TRUE;
}


BOOLEAN PAGEFRAME_INIT() {
    if (pageFrameInfo.initialized) return TRUE;
    if (!E820_INIT()) return FALSE;
    if (!READ_E820_MEMORYMAP()) return FALSE;
    pageFrameInfo.initialized = TRUE;
    return TRUE;
}

/* simple getters ---------------------------------------------------------*/

U32 GET_FREE_RAM() {
    return pageFrameInfo.freeMemory;
}
U32 GET_USED_RAM() {
    return pageFrameInfo.usedMemory;
}
U32 GET_RESERVED_RAM() {
    return pageFrameInfo.reservedMemory;
}

/* page allocator ---------------------------------------------------------*/


VOIDPTR REQUEST_PAGE() {
    U32 total_pages = pageFrameBitmap.size * 8;
    for (; pageFrameInfo.pIndex < total_pages; pageFrameInfo.pIndex++) {
        if (BITMAP_GET(&pageFrameBitmap, pageFrameInfo.pIndex)) continue; // in use/reserved

        // lock and return page
        LOCK_PAGE((VOIDPTR)(pageFrameInfo.pIndex * PAGE_SIZE));
        return (VOIDPTR)(pageFrameInfo.pIndex * PAGE_SIZE);
    }
    // no pages found, reset index and return NULL
    RESET_PAGEINDEX();
    return NULL;
}


/* Ensure heaps are initialized */
void kernel_heap_init() {
    if (heap_start) return; // already initialized
    heap_start = (KERNEL_HEAP_BLOCK *)MEM_RTOSKRNL_HEAP_BASE;
    heap_start->size = MEM_RTOSKRNL_HEAP_END - MEM_RTOSKRNL_HEAP_BASE;
    heap_start->free = TRUE;
    heap_start->next = NULL;
}

void user_heap_init() {
    if (user_heap_start) return;

    U32 heap_size = MEM_USER_SPACE_END_MIN - MEM_USER_SPACE_BASE;
    U32 heap_pages = pages_from_bytes(heap_size);
    // Map all user heap pages first
    for (U32 i = 0; i < heap_pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) panic("Out of physical memory for user heap", PANIC_OUT_OF_MEMORY);
        
        MAP_USER_PAGE((VOIDPTR)(MEM_USER_SPACE_BASE + i*PAGE_SIZE), phys,
                    PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }

    // Now it's safe to initialize the header
    user_heap_start = (USER_HEAP_BLOCK*)MEM_USER_SPACE_BASE;
    user_heap_start->size = heap_size;
    user_heap_start->free = TRUE;
    user_heap_start->next = NULL;
}



/* Request additional pages if heap block runs out */
static BOOLEAN expand_heap_user(USER_HEAP_BLOCK *last, U32 required_size) {
    // Align required size to page boundary
    U32 required_pages = pages_from_bytes(required_size);
    U32 alloc_size = required_pages * PAGE_SIZE;

    // Compute aligned vaddr for the new block
    U32 vaddr = (U32)last + last->size;
    vaddr = (vaddr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); 

    if (vaddr < MEM_USER_SPACE_BASE || vaddr + alloc_size >= MEM_USER_SPACE_END_MIN) {
        return FALSE; // out of user space
    }

    // Map pages
    for (U32 i = 0; i < required_pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) return FALSE; // out of physical memory

        MAP_USER_PAGE((VOIDPTR)(vaddr + i*PAGE_SIZE), phys,
                      PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }

    // Install new block header
    USER_HEAP_BLOCK *block = (USER_HEAP_BLOCK*)vaddr;
    block->size = alloc_size;
    block->free = TRUE;
    block->next = NULL;
    last->next = block;

    return TRUE;
}


static BOOLEAN expand_heap_kernel(KERNEL_HEAP_BLOCK *last, U32 required_size) {
    if ((U8*)last + last->size + required_size > (U8*)MEM_RTOSKRNL_HEAP_END) return FALSE;
    return TRUE;
}



/* --------------------------------------------------------------------------
   Allocation
-----------------------------------------------------------------------------*/
VOIDPTR KMALLOC(U32 size) {
    if (size == 0) return NULL;

    U32 total_size = align8(size + sizeof(KERNEL_HEAP_BLOCK));
    KERNEL_HEAP_BLOCK *block = heap_start;
    KERNEL_HEAP_BLOCK *last = NULL;

    while (block) {
        if (block->free && block->size >= total_size) {
            // split block if possible
            if (block->size >= total_size + sizeof(KERNEL_HEAP_BLOCK) + 8) {
                KERNEL_HEAP_BLOCK *new_block = (KERNEL_HEAP_BLOCK *)((U8 *)block + total_size);
                new_block->size = block->size - total_size;
                new_block->free = TRUE;
                new_block->next = block->next;
                block->size = total_size;
                block->next = new_block;
            }
            block->free = FALSE;
            return (VOIDPTR)(block + 1);
        }
        last = block;
        block = block->next;
    }

    // No free block found
    if (!last) return NULL; // heap not initialized?

    // Check heap boundary
    if ((U8*)last + last->size + total_size > (U8*)MEM_RTOSKRNL_HEAP_END) return NULL;

    // Allocate at the end of last block
    block = (KERNEL_HEAP_BLOCK *)((U8*)last + last->size);
    block->size = total_size;
    block->free = FALSE;
    block->next = NULL;
    last->next = block;

    return (VOIDPTR)(block + 1);
}


VOIDPTR UMALLOC(U32 size) {
    if (size == 0) return NULL;

    U32 total_size = align8(size + sizeof(USER_HEAP_BLOCK));
    USER_HEAP_BLOCK *block = user_heap_start;
    USER_HEAP_BLOCK *last = NULL;

    while (block) {
        if (block->free && block->size >= total_size) {
            // split block if possible
            if (block->size >= total_size + sizeof(USER_HEAP_BLOCK) + 8) {
                USER_HEAP_BLOCK *new_block = (USER_HEAP_BLOCK *)((U8 *)block + total_size);
                new_block->size = block->size - total_size;
                new_block->free = TRUE;
                new_block->next = block->next;
                block->size = total_size;
                block->next = new_block;
            }

            block->free = FALSE;
            return (VOIDPTR)(block + 1); // memory after header
        }
        last = block;
        block = block->next;
    }

    // Heap exhausted; try expanding
    if(!expand_heap_user(last, total_size)) return NULL;

    // allocate at last block end
    block = (USER_HEAP_BLOCK *)((U8 *)last + last->size);
    block->size = total_size;
    block->free = FALSE;
    block->next = NULL;
    last->next = block;

    return (VOIDPTR)(block + 1);
}

/* --------------------------------------------------------------------------
   Free
-----------------------------------------------------------------------------*/
VOID KFREE(VOIDPTR ptr) {
    if (!ptr) return;

    KERNEL_HEAP_BLOCK *block = ((KERNEL_HEAP_BLOCK *)ptr) - 1;
    block->free = TRUE;

    // coalesce next free blocks
    while (block->next && block->next->free) {
        block->size += block->next->size;
        block->next = block->next->next;
    }
}

VOID UFREE(VOIDPTR ptr) {
    if (!ptr) return;

    USER_HEAP_BLOCK *block = ((USER_HEAP_BLOCK *)ptr) - 1;
    block->free = TRUE;

    // coalesce next free blocks
    while (block->next && block->next->free) {
        block->size += block->next->size;
        block->next = block->next->next;
    }
}

/* --------------------------------------------------------------------------
   Calloc
-----------------------------------------------------------------------------*/
VOIDPTR KCALLOC(U32 num, U32 size) {
    U32 total = num * size;
    VOIDPTR ptr = KMALLOC(total);
    if (ptr) MEMSET(ptr, 0, total);
    return ptr;
}

VOIDPTR UCALLOC(U32 num, U32 size) {
    U32 total = num * size;
    VOIDPTR ptr = UMALLOC(total);
    if (ptr) MEMSET(ptr, 0, total);
    return ptr;
}
/* --------------------------------------------------------------------------
   Realloc
-----------------------------------------------------------------------------*/
BOOLEAN KREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize) {
    if (!addr) return FALSE;
    if (newSize == 0) {
        KFREE(*addr);
        *addr = NULL;
        return TRUE;
    }
    if (!*addr) {
        *addr = KMALLOC(newSize);
        return (*addr != NULL);
    }
    if (newSize <= oldSize) return TRUE;

    VOIDPTR newPtr = KMALLOC(newSize);
    if (!newPtr) return FALSE;

    MEMCPY(newPtr, *addr, oldSize);
    KFREE(*addr);
    *addr = newPtr;
    return TRUE;
}

BOOLEAN UREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize) {
    if (!addr) return FALSE;
    if (newSize == 0) {
        UFREE(*addr);
        *addr = NULL;
        return TRUE;
    }
    if (!*addr) {
        *addr = UMALLOC(newSize);
        return (*addr != NULL);
    }
    if (newSize <= oldSize) return TRUE;

    VOIDPTR newPtr = UMALLOC(newSize);
    if (!newPtr) return FALSE;

    MEMCPY(newPtr, *addr, oldSize);
    UFREE(*addr);
    *addr = newPtr;
    return TRUE;
}