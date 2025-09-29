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

#include <ERROR/ERROR.h>

#include <MEMORY/KMALLOC/KMALLOC.h>
#include <MEMORY/UMALLOC/UMALLOC.h>


static KERNEL_HEAP_BLOCK *heap_start = NULL;

static inline U32 align8(U32 size) {
    return (size + 7) & ~7U;
}

#define BYTE_TO_PAGE(x) ((x) / PAGE_SIZE)
__attribute__((section(".data")))
static BITMAP pageFrameBitmap = {0, 0};
__attribute__((section(".data")))
static PAGEFRAME_INFO pageFrameInfo = {0, 0, 0, 0, FALSE};
__attribute__((section(".data")))
static U8 bitmapData[MAX_BITMAP_SIZE] = {0}; // enough for largest physical memory

PAGEFRAME_INFO* GET_PAGEFRAME_INFO() {
    return &pageFrameInfo;
}
BITMAP* GET_PAGEFRAME_BITMAP() {
    return &pageFrameBitmap;
}



static void reserve_range(U32 start, U32 end) {
    U32 startFrame = start / PAGE_SIZE;
    U32 endFrame   = (end + PAGE_SIZE - 1) / PAGE_SIZE;
    for(U32 f = startFrame; f < endFrame; f++) {
        BITMAP_SET(&pageFrameBitmap, f, PAGE_ALLOCATED);
    }
}

static void unreserve_range(U32 start, U32 end) {
    U32 startFrame = start / PAGE_SIZE;
    U32 endFrame   = (end + PAGE_SIZE - 1) / PAGE_SIZE;
    for(U32 f = startFrame; f < endFrame; f++) {
        BITMAP_SET(&pageFrameBitmap, f, PAGE_FREE);
    }
}

// Create bitmap based on E820 info
BOOLEAN CREATE_PAGEFRAME_BITMAP() {
    E820Info *e820 = GET_E820_INFO();
    if (e820->RawEntryCount == 0) {
        return FALSE; // No E820 entries available
    }

    // Calculate total memory size from E820 entries
    U32 maxAddress = 0;
    for (U32 i = 0; i < e820->RawEntryCount; i++) {
        E820_ENTRY *entry = &e820->RawEntries[i];
        if (entry->Type == TYPE_E820_RAM) {
            U32 endAddress = entry->BaseAddressLow + entry->LengthLow;
            if (endAddress > maxAddress) {
                maxAddress = endAddress;
            }
        }
    }

    // Calculate number of pages
    U32 totalPages = pages_from_bytes(maxAddress);

    // Compute bitmap size in bytes
    U32 bitmapSizeBytes = (totalPages + 7) / 8; // Round up to nearest byte

    // Create bitmap
    BITMAP_CREATE(bitmapSizeBytes, bitmapData, &pageFrameBitmap);
    if (!pageFrameBitmap.data) {
        return FALSE; // Failed to create bitmap
    }

    // Mark all pages as free initially
    for(U32 i = 0; i < e820->RawEntryCount; i++) {
        E820_ENTRY *entry = &e820->RawEntries[i];
        if(entry->Type == TYPE_E820_RAM) {
            U32 startFrame = entry->BaseAddressLow / PAGE_SIZE;
            U32 endFrame   = (entry->BaseAddressLow + entry->LengthLow + PAGE_SIZE - 1) / PAGE_SIZE;
            for(U32 f = startFrame; f < endFrame; f++) {
                BITMAP_SET(&pageFrameBitmap, f, PAGE_FREE); // mark free
            }
        }
    }

    // Reserve critical regions
    reserve_range(0x00000000, 0x000FFFFF); // below 1MB
    reserve_range((U32)MEM_KRNL_ENTRY_BASE, (U32)MEM_KRNL_ENTRY_END); // kernel itself
    reserve_range(E820_TABLE_PHYS, E820_TABLE_END);         // E820 table
    VESA_INFO *vesa = GET_VESA_INFO();
    VBE_MODEINFO *vbe = GET_VBE_MODE();
    reserve_range(MEM_VESA_BASE, VBE_MODE_LOAD_ADDRESS_PHYS + sizeof(VBE_MODEINFO));
    reserve_range(vbe->PhysBasePtr, vbe->PhysBasePtr + (vbe->YResolution * vbe->BytesPerScanLineLinear));
    reserve_range(vbe->PhysBasePtr2, vbe->PhysBasePtr2 + (vbe->YResolution * vbe->BytesPerScanLineLinear));
    reserve_range(MEM_FRAMEBUFFER_BASE, MEM_FRAMEBUFFER_END); // video buffer
    reserve_range(MEM_RTOSKRNL_HEAP_BASE, MEM_RTOSKRNL_HEAP_END); // kernel heap
    reserve_range(MEM_BIOS_BASE, MEM_BIOS_END);
    reserve_range(MEM_RESERVED_BASE, MEM_RESERVED_END); // reserved MMIO / firmware
    return TRUE;
}

BOOLEAN CREATE_PAGEFRAME() {
    pageFrameInfo.freeMemory = 0;
    U32 totalFrames = pageFrameBitmap.size * 8;
    for(U32 i = 0; i < totalFrames; i++) {
        if(!BITMAP_GET(&pageFrameBitmap, i))
            pageFrameInfo.freeMemory += PAGE_SIZE;
    }
    pageFrameInfo.usedMemory = (totalFrames * PAGE_SIZE) - pageFrameInfo.freeMemory;
    pageFrameInfo.reservedMemory = pageFrameInfo.usedMemory;

    return TRUE;
}

BOOLEAN PAGEFRAME_INIT() {
    if (pageFrameInfo.initialized) {
        return TRUE; // Already initialized
    }
    if(!E820_INIT()) {
        return FALSE;
    }
    if(!CREATE_PAGEFRAME_BITMAP()) {
        return FALSE;
    }
    if(!CREATE_PAGEFRAME()) {
        return FALSE;
    }
    pageFrameInfo.initialized = TRUE;
    return TRUE;
}

ADDR GET_FREE_RAM() {
    return pageFrameInfo.freeMemory;
}

ADDR GET_USED_RAM() {
    return pageFrameInfo.usedMemory;
}

ADDR GET_RESERVED_RAM() {
    return pageFrameInfo.reservedMemory;
}

ADDR REQUEST_PAGE() {
    BITMAP *bmp = &pageFrameBitmap;
    PAGEFRAME_INFO *info = &pageFrameInfo;

    for(U32 i = info->pIndex; i < bmp->size * 8; i++) {
        if(!BITMAP_GET(bmp, i)) { // free page
            BITMAP_SET(bmp, i, PAGE_ALLOCATED); // mark as used
            info->freeMemory -= PAGE_SIZE;
            info->usedMemory += PAGE_SIZE;
            info->pIndex = i + 1; // next search starts here
            return i * PAGE_SIZE;
        }
    }
    return 0; // no free pages
}

ADDR REQUEST_PAGES(U32 numPages) {
    if (numPages == 0) return 0;
    BITMAP *bmp = &pageFrameBitmap;
    PAGEFRAME_INFO *info = &pageFrameInfo;

    U32 consecutive = 0;
    U32 startIndex = 0;

    for(U32 i = info->pIndex; i < bmp->size * 8; i++) {
        if(!BITMAP_GET(bmp, i)) { // free page
            if (consecutive == 0) {
                startIndex = i; // potential start of block
            }
            consecutive++;
            if (consecutive == numPages) {
                // Found a block
                for (U32 j = startIndex; j < startIndex + numPages; j++) {
                    BITMAP_SET(bmp, j, PAGE_ALLOCATED); // mark as used
                }
                info->freeMemory -= (numPages * PAGE_SIZE);
                info->usedMemory += (numPages * PAGE_SIZE);
                info->pIndex = startIndex + numPages; // next search starts here
                return startIndex * PAGE_SIZE;
            }
        } else {
            consecutive = 0; // reset count
        }
    }
    return 0; // no suitable block found
}

VOID FREE_PAGE(ADDR addr) {
    BITMAP *bmp = &pageFrameBitmap;
    PAGEFRAME_INFO *info = &pageFrameInfo;

    U32 pageIndex = addr / PAGE_SIZE;
    if (pageIndex >= bmp->size * 8) {
        return; // Out of bounds
    }
    U32 bitmap_val = BITMAP_GET(bmp, pageIndex);
    if(!bitmap_val ) {
        SET_ERROR_CODE(ERROR_DOUBLE_PAGE_FREE);
        return; // Already free
    }
    if(bitmap_val == PAGE_RESERVED || bitmap_val == PAGE_LOCKED) {
        SET_ERROR_CODE(bitmap_val == PAGE_LOCKED ? ERROR_FREEING_OF_LOCKED_PAGE : ERROR_FREEING_OF_RESERVED_PAGE);
        return; // Cannot free reserved or locked page
    }
    BITMAP_SET(bmp, pageIndex, PAGE_FREE); // Mark as free
    info->freeMemory += PAGE_SIZE;
    info->usedMemory -= PAGE_SIZE;
    if (pageIndex < info->pIndex) {
        info->pIndex = pageIndex; // Update pIndex for next search
    }
}

VOID FREE_PAGES(ADDR addr, U32 numPages) {
    for (U32 i = 0; i < numPages; i++) {
        FREE_PAGE(addr + (i * PAGE_SIZE));
    }
}

VOID RESET_PAGEINDEX() {
    pageFrameInfo.pIndex = 0;
}

VOID LOCK_PAGE(ADDR addr) {
    U32 pageIndex = addr / PAGE_SIZE;
    if (pageIndex >= pageFrameBitmap.size * 8) {
        return; // Out of bounds
    }
    if (!BITMAP_GET(&pageFrameBitmap, pageIndex)) { // only if free
        BITMAP_SET(&pageFrameBitmap, pageIndex, PAGE_LOCKED); // Mark as used
        pageFrameInfo.freeMemory -= PAGE_SIZE;
        pageFrameInfo.usedMemory += PAGE_SIZE;
    }
}

VOID LOCK_PAGES(ADDR addr, U32 numPages) {
    for (U32 i = 0; i < numPages; i++) {
        LOCK_PAGE(addr + (i * PAGE_SIZE));
    }
}

VOID UNLOCK_PAGE(ADDR addr) {
    U32 pageIndex = addr / PAGE_SIZE;
    if (pageIndex >= pageFrameBitmap.size * 8) {
        return; // Out of bounds
    }
    if (BITMAP_GET(&pageFrameBitmap, pageIndex) == PAGE_LOCKED) { // only if locked
        BITMAP_SET(&pageFrameBitmap, pageIndex, PAGE_FREE); // Mark as free
        pageFrameInfo.freeMemory += PAGE_SIZE;
        pageFrameInfo.usedMemory -= PAGE_SIZE;
    }
}

VOID UNLOCK_PAGES(ADDR addr, U32 numPages) {
    for (U32 i = 0; i < numPages; i++) {
        UNLOCK_PAGE(addr + (i * PAGE_SIZE));
    }
}

VOID RESERVE_PAGE(ADDR addr) {
    U32 pageIndex = addr / PAGE_SIZE;
    if (pageIndex >= pageFrameBitmap.size * 8) {
        return; // Out of bounds
    }
    if (!BITMAP_GET(&pageFrameBitmap, pageIndex)) { // only if free
        BITMAP_SET(&pageFrameBitmap, pageIndex, PAGE_RESERVED); // Mark as used
        pageFrameInfo.freeMemory -= PAGE_SIZE;
        pageFrameInfo.reservedMemory += PAGE_SIZE;
    }
}

VOID RESERVE_PAGES(ADDR addr, U32 numPages) {
    for (U32 i = 0; i < numPages; i++) {
        RESERVE_PAGE(addr + (i * PAGE_SIZE));
    }
}

VOID UNRESERVE_PAGE(ADDR addr) {
    U32 pageIndex = addr / PAGE_SIZE;
    if (pageIndex >= pageFrameBitmap.size * 8) {
        return; // Out of bounds
    }
    if (BITMAP_GET(&pageFrameBitmap, pageIndex) == PAGE_RESERVED) { // only if reserved
        BITMAP_SET(&pageFrameBitmap, pageIndex, PAGE_FREE); // Mark as free
        pageFrameInfo.freeMemory += PAGE_SIZE;
        pageFrameInfo.reservedMemory -= PAGE_SIZE;
    }
}

VOID UNRESERVE_PAGES(ADDR addr, U32 numPages) {
    for (U32 i = 0; i < numPages; i++) {
        UNRESERVE_PAGE(addr + (i * PAGE_SIZE));
    }
}
