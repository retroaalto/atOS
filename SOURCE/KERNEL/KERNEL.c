/*+++
    Source/KERNEL/KERNEL.c - 32-bit Kernel Entry Point

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit kernel entry point for atOS.

AUTHORS
    Antonako1
    
REVISION HISTORY
    2025/05/26 - Antonako1
        Initial version. Switched from KERNEL.asm to KERNEL.c.
    2025/08/18 - Antonako1
        Displays VESA/VBE, E820, GDT and IDT information.
        Initializes video mode and clears the screen.

REMARKS
    None
---*/
#include "./32RTOSKRNL/KERNEL.h"
#define ISO9660_ONLY_DEFINES
#include "./32RTOSKRNL/FS/ISO9660/ISO9660.h"
#include <PIT.h>
typedef struct {
    U32 ExtentLengthLE;
    U32 ExtentLocationLE_LBA;
} RTOSKRNL;

void normalize_path(U8 *path) {
    for (int i = 0; path[i]; i++) {
        if (path[i] == ';') {
            path[i] = '\0';
            break;
        }
    }
}

U32 __strlen(const U8 *str) {
    const U8 *s = str;
    while (*s) {
        s++;
    }
    return s - str;
}

BOOLEAN strcmp(const U8 *str1, const U8 *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (*str1 == *str2);
}

U8 *strncpy(U8 *dest, const U8 *src, U32 n) {
    U8 *d = dest;
    const U8 *s = src;
    while (n && *s) {
        *d++ = *s++;
        n--;
    }
    while (n--) {
        *d++ = 0;
    }
    return dest;
}

U32 CALC_SECTOR(U32 extent_length) {
    return (extent_length + 511) >> 9;
}

U0 *strchr(const U8 *str, U8 c) {
    while (*str) {
        if (*str == c) {
            return (U8 *)str;
        }
        str++;
    }
    return NULL;
}

U0 *memmove(void* dest, const void* src, U32 n) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;
    if (d < s) {
        for (U32 i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (U32 i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

U0 *memcpy(void* dest, const void* src, U32 n) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;
    for (U32 i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Searches for a file inside the ISO9660 filesystem's root directory
BOOLEAN SEARCH_FILE(
    U32 lba,
    U32 extentLength, 
    RTOSKRNL *rks, 
    U8 *target, 
    U8 *original_target,
    U0 *buffer, 
    U32 ATAPI_STATUS
) {
    U32 offset = 0;
    U8 record_name[21];

    // Load root directory record
    if(
        READ_CDROM(
            ATAPI_STATUS,
            lba,
            1,
            buffer
        ) == ATAPI_FAILED
    ) {
        return FALSE;
    }

    U32 i = 0;
    while(offset < extentLength) {
        IsoDirectoryRecord *record = (IsoDirectoryRecord *)((U8*)(buffer + offset));
        strncpy(record_name, record->fileIdentifier, record->fileNameLength);
        record_name[record->fileNameLength] = '\0';  // Null-terminate the string
        normalize_path(record_name);
        U8 test[20];
        memcpy(test, record->fileIdentifier, record->fileNameLength);
        test[record->fileNameLength] = '\0';
        if(record->length == 0) break;
        if (!(record->fileFlags & 0b00000010)) {
            // Inside a file
            if (strcmp(record_name, target)) {
                rks->ExtentLengthLE = record->extentLengthLE;
                rks->ExtentLocationLE_LBA = record->extentLocationLE_LBA;
                return TRUE;
            }
        } else {
            // Inside a directory
            if(strcmp(record_name, target)) {
                U8 *slash = strchr(original_target, '/');
                if(!slash) return FALSE;
                memmove(original_target, slash + 1, __strlen(slash));
                memcpy(target, original_target, __strlen(original_target));
                slash = strchr(target, '/');
                if (slash) {
                    *slash = '\0';
                } else {
                    target[__strlen(original_target)] = '\0';
                }
                return SEARCH_FILE(record->extentLocationLE_LBA, record->extentLengthLE, rks, target, original_target, buffer, ATAPI_STATUS);
            }
        }
        offset += record->length;
        i += VBE_CHAR_HEIGHT + 2;
    }

    return TRUE;
}

U0 kernel_after_gdt(U0);

__attribute__((noreturn))
void kernel_entry_main(U0) {
    CLI;
    vesa_check();
    vbe_check();
    GDT_INIT(); // Re-setup GDT just in case
    kernel_after_gdt();
    HLT;
}

U0 kernel_after_gdt(U0) {
    IDT_INIT();                       // Setup IDT
    SETUP_ISR_HANDLERS();
    IRQ_INIT();
    HLT;
    VBE_DRAW_LINE(1,1,1,200, VBE_GREEN); 
    VBE_UPDATE_VRAM();
    // todo: tss_init();
    STI;        // Enable interrupts
    VBE_DRAW_LINE(2,2,1,200, VBE_AQUA); VBE_UPDATE_VRAM();
    HLT;
    U32 row = 0;
    U32 atapi_status;
    #define rowinc row += VBE_CHAR_HEIGHT + 2
    atapi_status = ATAPI_CHECK();
    if (atapi_status == ATAPI_FAILED) {
        VBE_DRAW_STRING(0, row, "ATAPI check failed", VBE_WHITE, VBE_BLACK);
        VBE_UPDATE_VRAM();
        rowinc;
        HLT;
    }


    
    // Read ATOS/32RTOSKR.BIN;1 from disk
    // We will read the binary with ATAPI operations, 
    // not with DISK/ISO9660, to save the binary size of this file
    // Read buffer address will be at MEM_RESERVED_BASE
    U0 *RTOSKRNL_ADDRESS = (U0*)MEM_RTOSKRNL_BASE; // Main kernel address
    PrimaryVolumeDescriptor *pvd = (PrimaryVolumeDescriptor*)MEM_RESERVED_BASE;
    // Read buffer will be just after PVD in memory
    U8 *read_buffer = (U8*)(MEM_RESERVED_BASE + 2048);
    // Buffers limited to 21 to save binary size
    U8 filename[21] = "ATOS\0"; 
    U8 original_target[21] = "ATOS/32RTOSKR.BIN\0";

    RTOSKRNL rks = { 0 };
    // Read PVD at lba 16
    if(READ_CDROM(atapi_status, 16, 1, (U8*)pvd) == ATAPI_FAILED) {
        VBE_DRAW_STRING(0, row, "CDROM read failed", VBE_WHITE, VBE_BLACK);
        VBE_UPDATE_VRAM();
        rowinc;
        HLT;
    }
    // check PVD legitimacy
    if(
        pvd->TypeCode != 1 ||
        pvd->standardIdentifier[0] != 'C' ||
        pvd->standardIdentifier[1] != 'D' ||
        pvd->standardIdentifier[2] != '0' ||
        pvd->standardIdentifier[3] != '0' ||
        pvd->standardIdentifier[4] != '1' ||
        pvd->version != 1
    ) {
        VBE_DRAW_STRING(0, row, "Invalid PVD", VBE_WHITE, VBE_BLACK);
        VBE_UPDATE_VRAM();
        rowinc;
        HLT;
    }

    if(!SEARCH_FILE(
        pvd->rootDirectoryRecord.extentLocationLE_LBA,
        pvd->rootDirectoryRecord.extentLengthLE,
        &rks,
        filename,
        original_target,
        read_buffer,
        atapi_status
    )
    ) {
        rowinc;
        VBE_DRAW_STRING(0, row, "File not found", VBE_WHITE, VBE_BLACK);
        VBE_UPDATE_VRAM();
        rowinc;
        HLT;
    }

    U32 sectors = CALC_SECTOR(rks.ExtentLengthLE);
    if(
        READ_CDROM(
            atapi_status,
            rks.ExtentLocationLE_LBA,
            sectors,
            (U8*)RTOSKRNL_ADDRESS
        ) == ATAPI_FAILED
    ) {
        VBE_DRAW_STRING(0, row, "Failed to read 32RTOSKRNL", VBE_WHITE, VBE_BLACK);
        VBE_UPDATE_VRAM();
        rowinc;
        HLT;
    }

    __asm__ volatile ("jmp %0" : : "r"((U32)RTOSKRNL_ADDRESS));
    HLT;
}
    
__attribute__((noreturn, section(".text")))
void _start(void) {
    kernel_entry_main();
}
