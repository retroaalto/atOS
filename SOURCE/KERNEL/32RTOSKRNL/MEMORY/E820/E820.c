#include <MEMORY/E820/E820.h>
#include <VIDEO/VBE.h>
#include <STD/MEM.h>

#include <STD/STRING.h>

#define EXPECTED_MAX_SIZE 32
#define MIN_USER_SPACE_SIZE (MEM_USER_SPACE_END_MIN - MEM_USER_SPACE_BASE)

#define TYPE_E820_RAM 0x01
#define TYPE_E820_RESERVED 0x02
#define TYPE_E820_ACPI_RECLAIMABLE 0x03
#define TYPE_E820_ACPI_NVS 0x04

// Do NOT modify ever!
static U32 e820_entry_count;
static E820_ENTRY e820_entries[EXPECTED_MAX_SIZE];
static E820Info g_E820Info;

// Helper function to check if a 32-bit value fits in 32 bits
static inline BOOLEAN fits_in_32(U32 hi) { return hi == 0; }

E820Info *GET_E820_INFO(VOID) {
    return &g_E820Info;
}
E820_ENTRY *GET_E820_ENTRIES(VOID) {
    return e820_entries;
}
E820_ENTRY *GET_E820_ENTRY(U32 index) {
    if (index >= e820_entry_count) return NULL;
    return &e820_entries[index];
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

        U8 buf[50];
        ITOA_U(entry_table[i].BaseAddressLow, buf, 16);
        VBE_DRAW_STRING(0, 10 * i + 2, buf, VBE_GREEN, VBE_SEE_THROUGH);
        ITOA_U(entry_table[i].Type, buf, 16);
        VBE_DRAW_STRING(75, 10 * i + 2, buf, VBE_GREEN, VBE_SEE_THROUGH);
        ITOA_U(entry_table[i].LengthLow, buf, 16);
        VBE_DRAW_STRING(100, 10 * i + 2, buf, VBE_GREEN, VBE_SEE_THROUGH);
        VBE_UPDATE_VRAM();
        e820_entries[entries++] = entry_table[i];
    }
    e820_entry_count = entries;

    // Find an entry that:
    //  - has Type == RAM
    //  - base <= MEM_USER_SPACE_BASE
    //  - (base + length) >= (MEM_USER_SPACE_BASE + MIN_USER_SPACE_SIZE)
    U32 found_idx = U32_MAX;
    g_E820Info.RawEntryCount = 0;
    for (U32 i = 0; i < e820_entry_count; i++) {
        if(e820_entries[i].Type != TYPE_E820_RAM) continue;
        if (!fits_in_32(e820_entries[i].BaseAddressHigh) ||
            !fits_in_32(e820_entries[i].LengthHigh)) continue;

        g_E820Info.RawEntries[g_E820Info.RawEntryCount++] = e820_entries[i];
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
        ? heap_end : MEM_USER_SPACE_END_MIN;
    g_E820Info.TotalHeap = g_E820Info.HeapEndAddress - g_E820Info.HeapStartAddress;
    g_E820Info.FreeHeap = g_E820Info.TotalHeap;

    return TRUE;
}

