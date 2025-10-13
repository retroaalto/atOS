/*+++
    SOURCE/KERNEL/32RTOSKRNL/MEMORY/HEAP/KHEAP.h - Kernel Heap Management

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Kernel heap memory allocation functions.
    Kernel heap is a linear allocator, meaning that it allocates memory in a contiguous block.
    It does not support freeing individual allocations, only freeing all allocations at once.


AUTHORS
    Name(s)

REVISION HISTORY
    yyyy/mm/dd - Name(s)
        Description

REMARKS
    Additional remarks, if any.
---*/
#ifndef KHEAP_H
#define KHEAP_H
#include <STD/TYPEDEF.h>
#include <MEMORY/MEMORY.h>

/// @brief Allocates memory from the kernel heap.
/// @param size Size in bytes to allocate.
/// @return Pointer to the allocated memory, or NULL if allocation failed.
VOIDPTR KMALLOC(U32 size);

/// @brief Frees memory allocated from the kernel heap.
/// @param ptr Pointer to the memory to free.
/// @return None.
VOID KFREE(VOIDPTR ptr);

/// @brief Allocates zero-initialized memory from the kernel heap.
/// @param num Number of elements.
/// @param size Size of each element in bytes.
/// @return Pointer to the allocated memory, or NULL if allocation failed.
VOIDPTR KCALLOC(U32 num, U32 size);

/// @brief Reallocates memory in the kernel heap.
/// @param addr Pointer to the current memory block. Updated to point to the new block.
/// @param oldSize Current size of the memory block in bytes.
/// @param newSize New size of the memory block in bytes.
/// @return TRUE if reallocation succeeded, FALSE otherwise.
BOOLEAN KREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize);

/// @brief Allocates memory from the kernel heap with specified alignment.
/// @param size Size in bytes to allocate.
/// @param alignment Alignment in bytes (must be a power of two).
/// @return Pointer to the allocated memory, or NULL if allocation failed.
VOIDPTR KMALLOC_ALIGN(U32 size, U32 alignment);

/// @brief Frees memory allocated with KMALLOC_ALIGN.
/// @param ptr Pointer to the memory to free.
/// @return None.
VOID KFREE_ALIGN(VOIDPTR ptr);

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#define KHEAP_MAX_SIZE (MEM_KERNEL_HEAP_END - MEM_KERNEL_HEAP_BASE) // Maximum heap size in bytes
#define KHEAP_MAX_SIZE_PAGES (KHEAP_MAX_SIZE / PAGE_SIZE)
#define KHEAP_DEFAULT_SIZE_PAGES (KHEAP_MAX_SIZE_PAGES / 1.5) // Default initial heap size (2/3 of max)

/// @brief Initializes the kernel heap.
/// @param pageNum Number of pages to allocate for the heap, must be >= KHEAP_MIN_SIZE_PAGES.
/// @return TRUE if initialization succeeded, FALSE otherwise.
BOOLEAN KHEAP_INIT(U32 pageNum);

typedef struct KHeapBlock {
    U32 size;       // Size of the block (excluding header)
    U8 free;        // 1 = free, 0 = allocated
} KHeapBlock;

typedef struct {
    U32 totalSize;
    U32 usedSize;
    U32 freeSize;
    VOIDPTR baseAddress;
    VOIDPTR currentPtr;
} KHeap;

KHeap* KHEAP_GET_INFO(VOID);
KHeapBlock* KHEAP_GET_X_BLOCK(U32 index);
BOOLEAN KHEAP_EXPAND(U32 additionalPages);

#endif // KHEAP_H