#include <MEMORY/HEAP/KHEAP.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <ERROR/ERROR.h>
#include <STD/MEM.h>
#include <MEMORY/MEMORY.h>

/*+++

Kernel Heap Management

---*/

#define KHEAP_ALIGN 8u
#define ALIGN_UP_U32(x, a) ( ((x) + ((a) - 1u)) & ~((a) - 1u) )

// Minimum user-allocatable tail after splitting (avoid tiny fragments)
#define MIN_SPLIT_TAIL (sizeof(KHeapBlock) + 8u)


static KHeap kernelHeap __attribute__((section(".data"))) = {0, 0, 0, NULL, NULL};
static U32 heap_initialized __attribute__((section(".data"))) = FALSE;
static U32 heap_init_pages __attribute__((section(".data"))) = 0;

static inline U8 *kheap_base_ptr(void) { return (U8*)kernelHeap.baseAddress; }
static inline U8 *kheap_end_ptr(void)  { return (U8*)kernelHeap.baseAddress + kernelHeap.totalSize; }

BOOLEAN KHEAP_INIT(U32 pageNum) {
    if (heap_initialized) return TRUE;
    if (pageNum == 0) return FALSE;

    U32 heap_total_pages = KHEAP_MAX_SIZE_PAGES;
    if (pageNum > heap_total_pages) {
        SET_ERROR_CODE(ERROR_KHEAP_OUT_OF_MEMORY);
        return FALSE;
    }
    heap_init_pages = pageNum;

    // Make sure the region is available to us: turn RESERVED into LOCKED (allocated)
    // If you prefer, you can UNRESERVE then LOCK — but don’t leave it FREE, or other allocators may take it.
    KLOCK_PAGES(MEM_KERNEL_HEAP_BASE, pageNum);

    kernelHeap.baseAddress = (VOIDPTR)MEM_KERNEL_HEAP_BASE; // identity-mapped
    kernelHeap.totalSize   = pageNum * PAGE_SIZE;
    kernelHeap.usedSize    = 0;
    kernelHeap.freeSize    = kernelHeap.totalSize;
    kernelHeap.currentPtr  = kernelHeap.baseAddress;

    // Initialize the first free block
    KHeapBlock *block = (KHeapBlock*)kernelHeap.baseAddress;
    block->size = kernelHeap.totalSize - sizeof(KHeapBlock);
    block->free = 1;

    heap_initialized = TRUE;
    return TRUE;
}

BOOLEAN KHEAP_EXPAND(U32 additionalPages) {
    if (!heap_initialized || additionalPages == 0) return FALSE;

    U32 heap_total_pages = (MEM_KERNEL_HEAP_END - MEM_KERNEL_HEAP_BASE + PAGE_SIZE - 1) / PAGE_SIZE;
    if (heap_init_pages + additionalPages > heap_total_pages) {
        SET_ERROR_CODE(ERROR_KHEAP_OUT_OF_MEMORY);
        return FALSE;
    }

    // Lock additional pages
    KLOCK_PAGES(MEM_KERNEL_HEAP_BASE + heap_init_pages * PAGE_SIZE, additionalPages);
    kernelHeap.totalSize += additionalPages * PAGE_SIZE;
    kernelHeap.freeSize  += additionalPages * PAGE_SIZE;
    heap_init_pages += additionalPages;

    // Create a new free block at the end of the current heap
    KHeapBlock *newBlock = (KHeapBlock*)((U8*)kernelHeap.baseAddress + kernelHeap.totalSize - additionalPages * PAGE_SIZE);
    newBlock->size = additionalPages * PAGE_SIZE - sizeof(KHeapBlock);
    newBlock->free = 1;

    // Coalesce with previous block if free
    KHeapBlock *lastBlock = (KHeapBlock*)((U8*)newBlock - sizeof(KHeapBlock) - ((KHeapBlock*)((U8*)newBlock - sizeof(KHeapBlock)))->size);
    if ((U8*)lastBlock >= (U8*)kernelHeap.baseAddress && lastBlock->free) {
        lastBlock->size += sizeof(KHeapBlock) + newBlock->size;
    }

    return TRUE;
}

KHeap* KHEAP_GET_INFO(VOID) {
    if (!heap_initialized) return NULLPTR;
    return &kernelHeap;
}

KHeapBlock* KHEAP_GET_X_BLOCK(U32 index) {
    if (!heap_initialized) return NULLPTR;

    U8 *heapEnd = kheap_end_ptr();
    KHeapBlock *block = (KHeapBlock*)kernelHeap.baseAddress;
    U32 currentIndex = 0;

    while ((U8*)block < heapEnd) {
        if (currentIndex == index) {
            return block;
        }
        // Move to next block
        U8 *nextAddr = (U8*)block + sizeof(KHeapBlock) + block->size;
        if (nextAddr >= heapEnd) break; // reached end
        block = (KHeapBlock*)nextAddr;
        currentIndex++;
    }

    return NULLPTR; // index out of range
}

VOIDPTR KMALLOC(U32 size) {
    if (!size || !kernelHeap.baseAddress) return NULLPTR;

    // align request
    U32 req = ALIGN_UP_U32(size, KHEAP_ALIGN);

    U8 *heapEnd = kheap_end_ptr();
    KHeapBlock *start = (KHeapBlock*)kernelHeap.currentPtr;
    KHeapBlock *block = start;

    // guard: ensure start is within heap
    if ((U8*)start < kheap_base_ptr() || (U8*)start >= heapEnd) {
        // reset to base if currentPtr somehow out of range
        start = (KHeapBlock*)kernelHeap.baseAddress;
        block = start;
    }

    do {
        // Basic bounds check before reading header
        if ((U8*)block < kheap_base_ptr() || (U8*)block + sizeof(KHeapBlock) > heapEnd) {
            // Corrupt header or reached end; wrap to base and continue
            block = (KHeapBlock*)kernelHeap.baseAddress;
            if (block == start) break;
            continue;
        }

        if (block->free && block->size >= req) {
            // Can we split? Ensure the remaining tail can hold header + some space
            if (block->size >= req + MIN_SPLIT_TAIL) {
                KHeapBlock *next = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + req);
                // bounds check for next header
                if ((U8*)next + sizeof(KHeapBlock) <= heapEnd) {
                    next->size = block->size - req - sizeof(KHeapBlock);
                    next->free = 1;
                    block->size = req;
                }
                // if next header would cross end we skip splitting (treat as exact fit)
            }

            block->free = 0;
            kernelHeap.usedSize += block->size + sizeof(KHeapBlock);
            if (kernelHeap.freeSize >= block->size + sizeof(KHeapBlock))
                kernelHeap.freeSize -= block->size + sizeof(KHeapBlock);
            else
                kernelHeap.freeSize = 0; // safety

            // Move currentPtr to next block after allocated one
            U8 *nextPtr = (U8*)block + sizeof(KHeapBlock) + block->size;
            if (nextPtr >= heapEnd)
                kernelHeap.currentPtr = kernelHeap.baseAddress; // wrap around
            else
                kernelHeap.currentPtr = (VOIDPTR)nextPtr;

            return (VOIDPTR)((U8*)block + sizeof(KHeapBlock));
        }

        // advance to next block
        U8 *nextAddr = (U8*)block + sizeof(KHeapBlock) + block->size;
        if (nextAddr >= heapEnd) {
            // wrap-around
            block = (KHeapBlock*)kernelHeap.baseAddress;
        } else {
            block = (KHeapBlock*)nextAddr;
        }

    } while (block != start);

    SET_ERROR_CODE(ERROR_KHEAP_OUT_OF_MEMORY);
    return NULLPTR;
}

VOID KFREE(VOIDPTR ptr) {
    if (!ptr || kernelHeap.baseAddress == NULL) return;

    U8 *heapBase = kheap_base_ptr();
    U8 *heapEnd  = kheap_end_ptr();

    // Validate ptr and header location
    KHeapBlock *block = (KHeapBlock*)((U8*)ptr - sizeof(KHeapBlock));
    if ((U8*)block < heapBase || (U8*)block + sizeof(KHeapBlock) > heapEnd) return; // invalid pointer

    if (block->free) return; // already free

    block->free = 1;
    // adjust accounting with overflow-safety
    U32 reclaimed = block->size + sizeof(KHeapBlock);
    if (kernelHeap.usedSize >= reclaimed)
        kernelHeap.usedSize -= reclaimed;
    else
        kernelHeap.usedSize = 0;
    kernelHeap.freeSize += reclaimed;

    // Coalesce with next block if free and valid
    KHeapBlock *next = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + block->size);
    if ((U8*)next + sizeof(KHeapBlock) <= heapEnd) {
        if (next->free) {
            // ensure next->size is reasonable before adjusting
            block->size += sizeof(KHeapBlock) + next->size;
        }
    }

    // Coalesce with previous block (scan from base to find previous)
    // Only do this if block is not the first block at heap base
    if ((U8*)block > heapBase) {
        // scan blocks until we reach block (linear scan)
        KHeapBlock *scan = (KHeapBlock*)heapBase;
        while ((U8*)scan + sizeof(KHeapBlock) <= heapEnd) {
            U8 *scanNextAddr = (U8*)scan + sizeof(KHeapBlock) + scan->size;
            if (scanNextAddr == (U8*)block) {
                // found previous
                if (scan->free) {
                    // merge scan and block
                    scan->size += sizeof(KHeapBlock) + block->size;
                    // adjust accounting: we already marked `block` as free and added reclaimed,
                    // no further accounting needed here
                    block = scan; // merged block becomes the base
                }
                break;
            }
            if (scanNextAddr >= heapEnd) break;
            scan = (KHeapBlock*)scanNextAddr;
        }
    }
}

VOIDPTR KCALLOC(U32 num, U32 size) {
    // multiply with overflow check (simple)
    U32 total32 = (U32)num * (U32)size;
    if (total32 == 0 || total32 > 0xFFFFFFFFu) return NULLPTR;
    U32 total = (U32)total32;

    VOIDPTR ptr = KMALLOC(total);
    if (ptr) MEMSET(ptr, 0, total);
    return ptr;
}

BOOLEAN KREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize) {
    if (!addr || !*addr) return FALSE;

    U32 old_al = ALIGN_UP_U32(oldSize, KHEAP_ALIGN);
    U32 new_al = ALIGN_UP_U32(newSize, KHEAP_ALIGN);

    if (new_al <= old_al) return TRUE; // shrinking or same, nothing to do

    // try in-place expansion: see if next block is free and large enough
    U8 *heapBase = kheap_base_ptr();
    U8 *heapEnd  = kheap_end_ptr();

    KHeapBlock *block = (KHeapBlock*)((U8*)(*addr) - sizeof(KHeapBlock));
    if ((U8*)block >= heapBase && (U8*)block + sizeof(KHeapBlock) <= heapEnd) {
        KHeapBlock *next = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + block->size);
        if ((U8*)next + sizeof(KHeapBlock) <= heapEnd && next->free) {
            U32 combined = block->size + sizeof(KHeapBlock) + next->size;
            if (combined >= new_al) {
                // consume next block (possibly leaving a tail)
                U32 needed_extra = new_al - block->size;
                // If after consuming we can split tail, do so safely
                if (combined >= new_al + MIN_SPLIT_TAIL) {
                    // new next header after expansion
                    KHeapBlock *newNext = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + new_al);
                    if ((U8*)newNext + sizeof(KHeapBlock) <= heapEnd) {
                        newNext->size = combined - new_al - sizeof(KHeapBlock);
                        newNext->free = 1;
                    } else {
                        // cannot split, just grow into full combined block
                        new_al = combined;
                    }
                } else {
                    // take whole combined block
                    new_al = combined;
                }

                // update sizes & accounting
                U32 before = block->size + sizeof(KHeapBlock);
                block->size = new_al;
                U32 after = block->size + sizeof(KHeapBlock);
                // adjust used/free sizes
                if (after > before) {
                    U32 diff = after - before;
                    kernelHeap.usedSize += diff;
                    if (kernelHeap.freeSize >= diff) kernelHeap.freeSize -= diff;
                    else kernelHeap.freeSize = 0;
                }
                return TRUE;
            }
        }
    }

    // fallback: allocate new, copy, free old
    VOIDPTR newPtr = KMALLOC(newSize);
    if (!newPtr) return FALSE;

    MEMCPY(newPtr, *addr, oldSize);
    KFREE(*addr);
    *addr = newPtr;
    return TRUE;
}

VOIDPTR KMALLOC_ALIGN(U32 size, U32 alignment) {
    if (alignment == 0) alignment = KHEAP_ALIGN; // fallback
    // Allocate extra memory to store the original pointer for KFREE
    U32 totalSize = size + alignment + sizeof(VOIDPTR);

    VOIDPTR rawPtr = KMALLOC(totalSize);
    if (!rawPtr) return NULLPTR;

    // Align the pointer
    U8* raw = (U8*)rawPtr + sizeof(VOIDPTR);
    U8* aligned = (U8*)ALIGN_UP_U32((U32)raw, alignment);

    // Store the original pointer just before the aligned block
    ((VOIDPTR*)aligned)[-1] = rawPtr;

    return (VOIDPTR)aligned;
}

VOID KFREE_ALIGN(VOIDPTR ptr) {
    if (!ptr) return;

    // Retrieve the original pointer stored before aligned block
    VOIDPTR rawPtr = ((VOIDPTR*)ptr)[-1];
    KFREE(rawPtr);
}
