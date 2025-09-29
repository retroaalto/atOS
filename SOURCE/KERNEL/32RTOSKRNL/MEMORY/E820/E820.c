#include <MEMORY/E820/E820.h>
#include <VIDEO/VBE.h>
#include <STD/MEM.h>

#include <STD/STRING.h>

#define EXPECTED_MAX_SIZE 32
#define MIN_USER_SPACE_SIZE (MEM_USER_SPACE_END_MIN - MEM_USER_SPACE_BASE)



// Do NOT modify ever!
static U32 e820_entry_count;
static E820_ENTRY e820_entries[EXPECTED_MAX_SIZE];
static E820Info g_E820Info;

// Helper function to check if a 32-bit value fits in 32 bits

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

static inline BOOLEAN add_u32_would_overflow(U32 a, U32 b) {
    return b > (U32)(0xFFFFFFFFu - a);
}


// Reads all usable RAM entries from the E820 table into g_E820Info
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

    g_E820Info.RawEntryCount = 0;
    for (U32 i = 0; i < e820_entry_count; i++) {
        if(e820_entries[i].Type == TYPE_E820_RAM) {
            if (!fits_in_32(e820_entries[i].BaseAddressHigh) ||
                !fits_in_32(e820_entries[i].LengthHigh)) continue;
            if (g_E820Info.RawEntryCount < E820_MAX_ENTRIES) {
                g_E820Info.RawEntries[g_E820Info.RawEntryCount++] = e820_entries[i];
            }
        } else {
            continue; // Skip non-RAM entries
        }
    }
    if(g_E820Info.RawEntryCount == 0) {
        return FALSE; // No usable RAM entries found
    }
    return TRUE;
}