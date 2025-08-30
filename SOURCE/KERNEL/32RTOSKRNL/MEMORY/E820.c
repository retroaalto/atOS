#include <MEMORY/E820.h>
#include <VIDEO/VBE.h>
#include <STD/MEM.h>

#define EXPECTED_MAX_SIZE 32
#define MIN_USER_SPACE_SIZE (MEM_USER_SPACE_MIN - MEM_USER_SPACE_BASE)

#define SET_IN_USE(x)     ((x)->Flags |= 0b00000001)
#define CLEAR_IN_USE(x)   ((x)->Flags &= ~0b00000001)
#define IS_IN_USE(x)      (((x)->Flags & 0b00000001) != 0)
typedef struct __attribute__((packed)) {
    U32 Size;
    U32 *Address;
    U8 Flags;
    U32 Reserved;
    U32 *Next;
} MemBlock;

typedef struct __attribute__((packed)) {
    MemBlock *First;
    MemBlock *Last;
} MemBlockArray;

typedef struct {
    U32 HeapStartAddress;
    U32 HeapEndAddress;
    U32 TotalHeap;
    U32 FreeHeap;
    U32 AtTableIndex;
} E820Info;

typedef struct {
    U32 HeapStartAddress;
    U32 HeapEndAddress;
    U0 *NextFreeAddress;
} HeapState;


// Do NOT modify ever!
static U32 e820_entry_count;
static E820_ENTRY e820_entries[EXPECTED_MAX_SIZE];
static E820Info g_E820Info;

// Free to modify
static HeapState g_HeapState = {0};
static MemBlockArray g_MemBlocks = {0};
static MemBlockArray g_FreedMemBlocks = {0};

// Helper function to check if a 32-bit value fits in 32 bits
static inline BOOLEAN fits_in_32(U32 hi) { return hi == 0; }

VOID *MAlloc(U32 Size) {
    if(Size == 0 || Size > g_E820Info.FreeHeap) {
        return NULLPTR;
    }
    
    g_E820Info.FreeHeap -= Size;
    return (VOID *)0;
}

U32 GET_FREE_TOTAL_HEAP(VOID) {
    return g_E820Info.FreeHeap;
}

BOOLEAN E820_INIT(VOID) {
    E820_ENTRY *entry_table = (E820_ENTRY *)E820_TABLE_PHYS;
    U32 entries = 0;

    // Copy the E820 entries from the entry table to our local array
    for (U32 i = 0; i < EXPECTED_MAX_SIZE; i++) {
        if (entry_table[i].BaseAddressLow == 0 && entry_table[i].LengthLow == 0
            && entry_table[i].BaseAddressHigh == 0 && entry_table[i].LengthHigh == 0) {
            break;
        }
        e820_entries[entries++] = entry_table[i];
    }
    e820_entry_count = entries;

    // Find an entry that:
    //  - has Type == RAM
    //  - base <= MEM_USER_SPACE_BASE
    //  - (base + length) >= (MEM_USER_SPACE_BASE + MIN_USER_SPACE_SIZE)
    U32 found_idx = U32_MAX;
    for (U32 i = 0; i < e820_entry_count; i++) {
        if(e820_entries[i].Type != TYPE_E820_RAM) continue;
        if (!fits_in_32(e820_entries[i].BaseAddressHigh) ||
            !fits_in_32(e820_entries[i].LengthHigh)) continue;

        U32 base = (U32)e820_entries[i].BaseAddressLow;
        U32 len  = (U32)e820_entries[i].LengthLow;
        U32 region_end = base + len; // exclusive

        U32 needed_start = (U32)MEM_USER_SPACE_BASE;
        U32 needed_end   = needed_start + (U32)MIN_USER_SPACE_SIZE;

        if (base <= needed_start && region_end >= needed_end) {
            found_idx = i;
            break;
        }
    }
    // No suitable E820 entry found
    if(found_idx == U32_MAX) return FALSE;

    // calculate heap layout
    U32 heap_start = (U32)MEM_USER_SPACE_BASE;
    U32 heap_end = e820_entries[found_idx].BaseAddressLow + e820_entries[found_idx].LengthLow;

    if(heap_end <= heap_start) return FALSE;

    g_E820Info.AtTableIndex = found_idx;
    g_E820Info.HeapStartAddress = heap_start;
    g_E820Info.HeapEndAddress = 
        (heap_end > MEM_USER_SPACE_BASE + MIN_USER_SPACE_SIZE) 
        ? heap_end : MEM_USER_SPACE_MIN;
    g_E820Info.TotalHeap = g_E820Info.HeapEndAddress - g_E820Info.HeapStartAddress;
    g_E820Info.FreeHeap = g_E820Info.TotalHeap;

    g_HeapState.HeapStartAddress = g_E820Info.HeapStartAddress;
    g_HeapState.HeapEndAddress = g_E820Info.HeapEndAddress;
    g_HeapState.NextFreeAddress = (U32 *)g_HeapState.HeapStartAddress;

    g_MemBlocks.First = NULLPTR;
    g_MemBlocks.Last = NULLPTR;

    g_FreedMemBlocks.First = NULLPTR;
    g_FreedMemBlocks.Last = NULLPTR;

    return TRUE;
}


void MAllocReset(void) {
    g_HeapState.NextFreeAddress = g_HeapState.HeapStartAddress;
    g_E820Info.FreeHeap = g_HeapState.HeapEndAddress - (U32)g_HeapState.NextFreeAddress;
}