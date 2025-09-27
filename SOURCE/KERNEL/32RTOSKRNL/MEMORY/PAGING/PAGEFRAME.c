#include <MEMORY/PAGING/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <MEMORY/E820/E820.h>
#include <MEMORY/MEMORY.h>
#include <STD/MEM.h>
#include <STD/BINARY.h>
#include <STD/ASM.h>
#include <STD/BITMAP.h>
#include <VIDEO/VBE.h>

#define BYTE_TO_PAGE(x) ((x) / PAGE_SIZE)
typedef struct {
    U32 freeMemory; // in bytes
    U32 reservedMemory; // in bytes
    U32 usedMemory; // in bytes
    U32 pIndex; // page index
    BOOLEAN initialized; // boolean
} PAGEFRAME_INFO;

static BITMAP pageFrameBitmap;
static PAGEFRAME_INFO pageFrameInfo;

VOID RESET_PAGEINDEX() { pageFrameInfo.pIndex = 0; }

VOID FREE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    if(!BITMAP_GET(&pageFrameBitmap, index)) return;   // Already free
    if(!BITMAP_SET(&pageFrameBitmap, index, FALSE)) return;
    // Decide whether this was reserved or used
    // By design, FREE_PAGE should only be called on "locked" pages
    pageFrameInfo.freeMemory += PAGE_SIZE;
    pageFrameInfo.usedMemory -= PAGE_SIZE;
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
    if(BITMAP_GET(&pageFrameBitmap, index)) return;    // Already locked/reserved
    if(!BITMAP_SET(&pageFrameBitmap, index, TRUE)) return;
    pageFrameInfo.freeMemory -= PAGE_SIZE;
    pageFrameInfo.usedMemory += PAGE_SIZE;
}

VOID LOCK_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        LOCK_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

VOID RESERVE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    if(BITMAP_GET(&pageFrameBitmap, index)) return;    // Already reserved/used
    if(!BITMAP_SET(&pageFrameBitmap, index, TRUE)) return;
    pageFrameInfo.freeMemory -= PAGE_SIZE;
    pageFrameInfo.reservedMemory += PAGE_SIZE;
}

VOID RESERVE_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        RESERVE_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

VOID UNRESERVE_PAGE(VOIDPTR addr) {
    U32 index = (U32)addr / PAGE_SIZE;
    if(!BITMAP_GET(&pageFrameBitmap, index)) return;   // Already free
    if(!BITMAP_SET(&pageFrameBitmap, index, FALSE)) return;
    pageFrameInfo.freeMemory += PAGE_SIZE;
    pageFrameInfo.reservedMemory -= PAGE_SIZE;
    if(pageFrameInfo.pIndex > index) {
        pageFrameInfo.pIndex = index;
    }
}

VOID UNRESERVE_PAGES(VOIDPTR addr, U32 numPages) {
    for(U32 i = 0; i < numPages; i++) {
        UNRESERVE_PAGE((VOIDPTR)((U32)addr + (i * PAGE_SIZE)));
    }
}

BOOLEAN READ_E820_MEMORYMAP(VOID) {
    E820Info *info = GET_E820_INFO();
    if (!info) return FALSE;

    VOIDPTR largestFreeSeg = 0;
    U32 largestFreeSegSize = 0;

    // Get the largest free segment
    for (U32 i = 0; i < info->RawEntryCount; i++) {
        U32 segSize = (U32)info->RawEntries[i].LengthLow;
        if(segSize > largestFreeSegSize) {
            largestFreeSegSize = segSize;
            largestFreeSeg = (VOIDPTR)info->RawEntries[i].BaseAddressLow;
        }
    }

    U32 memorySize = 0;
    for(U32 i = 0; i < info->RawEntryCount; i++) {
        U32 end = (U32)(info->RawEntries[i].BaseAddressLow + info->RawEntries[i].LengthLow);
        if(end > memorySize) memorySize = end;
    }

    // Create a bitmap for the page frame
    U32 bitmap_size = memorySize / PAGE_SIZE / 8 + 1;
    BITMAP_CREATE(bitmap_size, (VOIDPTR)largestFreeSeg, &pageFrameBitmap);
    VBE_DRAW_LINE(0, 0, 1024, 0, VBE_GREEN); VBE_UPDATE_VRAM();

    // Reserve all pages in the bitmap
    RESERVE_PAGES(0, bitmap_size / PAGE_SIZE + 1);

    // Unreserve usable ram
    for(U32 i = 0; i < info->RawEntryCount; i++) {
        UNRESERVE_PAGES(
            (VOIDPTR)info->RawEntries[i].BaseAddressLow, 
            info->RawEntries[i].LengthLow / PAGE_SIZE
        );
    }

    // Lock BIOS+kernel low mem
    RESERVE_PAGES(0, 0x100);

    // Reserve E820
    RESERVE_PAGES(MEM_E820_BASE, ALIGN_UP(MEM_E820_END - MEM_E820_BASE, PAGE_SIZE) / PAGE_SIZE);
    // Reserve VESA and VBE
    RESERVE_PAGES(MEM_VESA_BASE, ALIGN_UP(MEM_VESA_END - MEM_VESA_BASE, PAGE_SIZE) / PAGE_SIZE);
    // Reserve VBE VRAM buffer
    U32 *VIDEO_BUFFER = GET_VBE_MODE()->PhysBasePtr;
    RESERVE_PAGES(VIDEO_BUFFER, ALIGN_UP(FRAMEBUFFER_SIZE, PAGE_SIZE) / PAGE_SIZE);
    // Reserve Framebuffer
    RESERVE_PAGES(MEM_FRAMEBUFFER_BASE, ALIGN_UP(FRAMEBUFFER_SIZE, PAGE_SIZE) / PAGE_SIZE);
    // Reserve BIOS reserved memory
    RESERVE_PAGES(MEM_BIOS_BASE, ALIGN_UP(MEM_BIOS_END - MEM_BIOS_BASE, PAGE_SIZE) / PAGE_SIZE);
    // Lock RTOS kernel
    LOCK_PAGES(MEM_RTOSKRNL_BASE, ALIGN_UP(MEM_RTOSKRNL_END - MEM_RTOSKRNL_BASE, PAGE_SIZE) / PAGE_SIZE);
    // Lock RTOS kernel heap
    LOCK_PAGES(
        (VOIDPTR)info->HeapStartAddress, 
        ALIGN_UP(info->HeapEndAddress - info->HeapStartAddress, PAGE_SIZE) / PAGE_SIZE
    );

    // Lock bitmap
    RESERVE_PAGES(
        (VOIDPTR)pageFrameBitmap.data, 
        ALIGN_UP(bitmap_size, PAGE_SIZE) / PAGE_SIZE
    );
    return TRUE;
}

BOOLEAN PAGEFRAME_INIT() {
    if(pageFrameInfo.initialized) return;
    pageFrameInfo.initialized = TRUE;
    if(!E820_INIT()) return FALSE;
    if(!READ_E820_MEMORYMAP()) return FALSE;
    return TRUE;
}

U32 GET_FREE_RAM() {
    return pageFrameInfo.freeMemory;
}
U32 GET_USED_RAM() {
    return pageFrameInfo.usedMemory;
}
U32 GET_RESERVED_RAM() {
    return pageFrameInfo.reservedMemory;
}
VOIDPTR REQUEST_PAGE() {
    for(; pageFrameInfo.pIndex < pageFrameBitmap.size * 8; pageFrameInfo.pIndex++) {
        if(BITMAP_GET(&pageFrameBitmap, pageFrameInfo.pIndex)) continue;
        LOCK_PAGE((VOIDPTR)(pageFrameInfo.pIndex * PAGE_SIZE));
        return (VOIDPTR)(pageFrameInfo.pIndex * PAGE_SIZE);
    }
    RESET_PAGEINDEX();
    return NULL;
}