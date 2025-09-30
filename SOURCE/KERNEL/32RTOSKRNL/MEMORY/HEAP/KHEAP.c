#include <MEMORY/HEAP/KHEAP.h>
#include <MEMORY/HEAP/UHEAP.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <ERROR/ERROR.h>
#include <STD/MEM.h>

/*+++

Kernel Heap Management

---*/

static KHeap kernelHeap __attribute__((section(".data"))) = {0, 0, 0, NULL, NULL};

BOOLEAN KHEAP_INIT(U32 pageNum) {
    if (pageNum == 0) return FALSE;

    
    ADDR heapAddr = REQUEST_PAGES(pageNum);
    if (heapAddr == 0) {
        SET_ERROR_CODE(ERROR_KHEAP_OUT_OF_MEMORY);
        return FALSE;
    }
    
    kernelHeap.baseAddress = (VOIDPTR)heapAddr;
    kernelHeap.totalSize = pageNum * PAGE_SIZE;
    kernelHeap.usedSize = 0;
    kernelHeap.freeSize = kernelHeap.totalSize;

    // Initialize the first free block
    KHeapBlock *block = (KHeapBlock*)kernelHeap.baseAddress;
    block->size = kernelHeap.totalSize - sizeof(KHeapBlock);
    block->free = 1;

    return TRUE;
}


VOIDPTR KMALLOC(U32 size) {
    if (!size || !kernelHeap.baseAddress) return NULLPTR;

    KHeapBlock *start = (KHeapBlock*)kernelHeap.currentPtr;
    KHeapBlock *block = start;

    do {
        if (block->free && block->size >= size) {
            // Split block if large enough
            if (block->size >= size + sizeof(KHeapBlock) + 1) {
                KHeapBlock *next = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + size);
                next->size = block->size - size - sizeof(KHeapBlock);
                next->free = 1;
                block->size = size;
            }

            block->free = 0;
            kernelHeap.usedSize += block->size + sizeof(KHeapBlock);
            kernelHeap.freeSize -= block->size + sizeof(KHeapBlock);

            // Move currentPtr to next block after allocated one
            kernelHeap.currentPtr = (U8*)block + sizeof(KHeapBlock) + block->size;
            if ((U32)(kernelHeap.currentPtr) >= (U32)((U32)kernelHeap.baseAddress + kernelHeap.totalSize))
                kernelHeap.currentPtr = kernelHeap.baseAddress; // wrap around

            return (VOIDPTR)((U8*)block + sizeof(KHeapBlock));
        }

        block = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + block->size);

        // Wrap around if end reached
        if ((U8*)block >= (U8*)kernelHeap.baseAddress + kernelHeap.totalSize)
            block = (KHeapBlock*)kernelHeap.baseAddress;

    } while (block != start);

    SET_ERROR_CODE(ERROR_KHEAP_OUT_OF_MEMORY);
    return NULLPTR;
}


VOID KFREE(VOIDPTR ptr) {
    if (!ptr || kernelHeap.baseAddress == NULL) return;

    KHeapBlock *block = (KHeapBlock*)((U8*)ptr - sizeof(KHeapBlock));
    if (block->free) return; // already free

    block->free = 1;
    kernelHeap.usedSize -= block->size + sizeof(KHeapBlock);
    kernelHeap.freeSize += block->size + sizeof(KHeapBlock);

    // Coalesce with next block if free
    KHeapBlock *next = (KHeapBlock*)((U8*)block + sizeof(KHeapBlock) + block->size);
    if ((U8*)next < (U8*)kernelHeap.baseAddress + kernelHeap.totalSize && next->free) {
        block->size += sizeof(KHeapBlock) + next->size;
    }
}

VOIDPTR KCALLOC(U32 num, U32 size) {
    U32 total = num * size;
    VOIDPTR ptr = KMALLOC(total);
    if (ptr) MEMSET(ptr, 0, total);
    return ptr;
}

BOOLEAN KREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize) {
    if (!addr || !*addr) return FALSE;

    if (newSize <= oldSize) return TRUE; // shrinking, nothing to do

    VOIDPTR newPtr = KMALLOC(newSize);
    if (!newPtr) return FALSE;

    MEMCPY(newPtr, *addr, oldSize);
    KFREE(*addr);
    *addr = newPtr;
    return TRUE;
}