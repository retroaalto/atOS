#include "./ISO9660.h"
#include "../MEMORY/MEMORY.h"
#include <STD/STRING.h>
#include <STD/MEM.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <DRIVERS/ATAPI/ATAPI.h>
#include <DRIVERS/VIDEO/VBE.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>

#define MAlloc(sz) KMALLOC(sz)
#define Free(ptr) KFREE(ptr)

static PrimaryVolumeDescriptor *pvd = NULLPTR;

void SetGlobalPVD(PrimaryVolumeDescriptor *descriptor) {
    if (!descriptor) return;
    if (pvd && pvd != descriptor) Free(pvd);
    pvd = descriptor;
}

BOOLEAN ISO9660_IMAGE_CHECK(PrimaryVolumeDescriptor *pvd) {
    if (!pvd) return FALSE;
    return STRNCMP(pvd->standardIdentifier, "CD001", 5);
}

BOOLEAN ISO9660_READ_PVD(PrimaryVolumeDescriptor *descriptor, U32 size) {
    if (!descriptor || size < sizeof(PrimaryVolumeDescriptor)) return FALSE;

    U32 atapiStatus = INITIALIZE_ATAPI();
    if (atapiStatus == ATA_FAILED) return FALSE;

    if (READ_CDROM(atapiStatus, 16, 1, descriptor) == ATA_FAILED) return FALSE;

    return TRUE;
}

CHAR *ISO9660_NORMALIZE_PATH(CHAR *path) {
    if (!path) return NULLPTR;
    U32 len = STRLEN(path);
    if (len == 0 || len >= ISO9660_MAX_PATH) return NULLPTR;

    CHAR *res = MAlloc(ISO9660_MAX_PATH);
    if (!res) return NULLPTR;

    U32 filenameLen = 0;
    U32 extensionLen = 0;
    BOOLEAN hasExtension = FALSE;
    U32 j = 0;

    for (U32 i = 0; i < len && j < ISO9660_MAX_PATH - 1; i++) {
        CHAR c = TOUPPER(path[i]);

        if (c == '/' || c == '\\') {
            res[j++] = '/';
            filenameLen = 0;
            extensionLen = 0;
            hasExtension = FALSE;
            continue;
        }

        if (c == '.') {
            if (hasExtension) {
                Free(res);
                return NULLPTR; // multiple dots not allowed
            }
            hasExtension = TRUE;
            extensionLen = 0;
            res[j++] = c;
            continue;
        }

        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            Free(res);
            return NULLPTR;
        }

        if (hasExtension) {
            if (extensionLen >= ISO9660_MAX_EXTENSION_LENGTH) continue;
            extensionLen++;
        } else {
            if (filenameLen >= ISO9660_MAX_FILENAME_LENGTH) continue;
            filenameLen++;
        }

        res[j++] = c;
    }

    res[j] = '\0';

    if (j < 2 || !(res[j - 2] == ';' && res[j - 1] == '1')) {
        if (j + 2 >= ISO9660_MAX_PATH) {
            Free(res);
            return NULLPTR;
        }
        res[j++] = ';';
        res[j++] = '1';
        res[j] = '\0';
    }

    return res;
}

void ISO9660_EXTRACT_ROOT_FROM_PVD(PrimaryVolumeDescriptor *pvd, IsoDirectoryRecord *root) {
    if (!pvd || !root) return;
    *root = pvd->rootDirectoryRecord;
}

U32 ISO9660_CALCULATE_SECTORS(U32 extent_length) {
    return (extent_length + ISO9660_SECTOR_SIZE - 1) / ISO9660_SECTOR_SIZE;
}

/* Recursive directory reading */
BOOLEAN ISO9660_READ_FROM_DIRECTORY(
    U32 lba,
    U32 size,
    CHAR *target,
    CHAR *original_target,
    IsoDirectoryRecord *output
) {
    if (!target || !original_target || !output) return FALSE;

    U32 sectors = ISO9660_CALCULATE_SECTORS(size);
    U8 *buffer = MAlloc(sectors * 2048);
    if (!buffer) return FALSE;

    if (READ_CDROM(GET_ATAPI_INFO(), lba, sectors, buffer) == ATA_FAILED) {
        Free(buffer);
        return FALSE;
    }

    U32 offset = 0;
    while (offset < size) {
        IsoDirectoryRecord *record = (IsoDirectoryRecord *)(buffer + offset);
        if (record->length == 0) break;
        if (record->fileNameLength >= ISO9660_MAX_PATH) {
            Free(buffer);
            return FALSE;
        }

        CHAR record_name[ISO9660_MAX_PATH];
        STRNCPY(record_name, record->fileIdentifier, record->fileNameLength);
        record_name[record->fileNameLength] = '\0';

        if (record->fileFlags & ISO9660_FILE_FLAG_DIRECTORY) {
            if (STRCMP(record_name, target)) {
                CHAR *slash = STRCHR(original_target, '/');
                if (slash) {
                    U32 remaining = STRLEN(slash + 1);
                    MEMMOVE(original_target, slash + 1, remaining);
                    original_target[remaining] = '\0';

                    MEMCPY(target, original_target, remaining + 1);
                    slash = STRCHR(target, '/');
                    if (slash) *slash = '\0';
                } else {
                    target[0] = '\0';
                }

                BOOLEAN res = ISO9660_READ_FROM_DIRECTORY(
                    record->extentLocationLE_LBA,
                    record->extentLengthLE,
                    target,
                    original_target,
                    output
                );
                Free(buffer);
                return res;
            }
        } else {
            if (STRCMP(record_name, target)) {
                MEMCPY(output, record, sizeof(IsoDirectoryRecord));
                Free(buffer);
                return TRUE;
            }
        }

        offset += record->length;
    }

    Free(buffer);
    return FALSE;
}

BOOLEAN ISO9660_PATH_TABLE_SEARCH(
    CHAR *original_target,
    IsoDirectoryRecord *output,
    IsoDirectoryRecord *root
) {
    if(!pvd) return FALSE;
    U32 path_table_size = pvd->pathTableSizeLE;
    U32 path_table_loc = pvd->pathTableLocationLE;
    U8 *buf = MAlloc(ATAPI_SECTOR_SIZE);
    if(!buf) return FALSE;
    BOOL res = READ_CDROM(GET_ATAPI_INFO(), path_table_loc, ATAPI_CALC_SECTORS(path_table_size), buf);
    if(!res) {
        Free(buf);
        return res;
    }
    U8 *target = MAlloc(STRLEN(original_target) + 1);
    if (!target) {
        Free(buf);
        return FALSE;
    };
    STRCPY(target, original_target);
    U8 *slash = (U8 *)STRCHR(target, '/');
    if (slash) *slash = '\0';

    PathTableEntry *ent = NULLPTR;
    U32 pos = 0;
    U32 look_for_parent_id = 1;//root id
    BOOL found_dir = FALSE;
    while(1) {
        ent = (PathTableEntry *)(buf + pos);

        if (STRNCMP(target, ent->name, ent->nameLength) == TRUE &&
            ent->parentDirNum == look_for_parent_id) {
            
            look_for_parent_id = ent->parentDirNum;

            // Check for remaining path
            CHAR *slash = STRCHR(original_target, '/');
            if (slash) {
                U32 remaining = STRLEN(slash + 1);
                MEMMOVE(original_target, slash + 1, remaining);
                original_target[remaining] = '\0';
                MEMCPY(target, original_target, remaining + 1);
                slash = STRCHR(target, '/');
                if (slash) *slash = '\0';
                pos = 0; // restart scanning path table
            } else {
                // Last component reached
                found_dir = TRUE;
                break;
            }
        }

        pos += sizeof(PathTableEntry) - 1 + ent->nameLength;
        if (ent->nameLength % 2 == 1) pos++;
        if(pos >= path_table_size) break;
    }
    Free(buf);
    if (!found_dir && STRLEN(target) == 0) {
        Free(target);
        return FALSE;
    }
    
    res = READ_CDROM(GET_ATAPI_INFO(), ent->lbaLocation, 1, buf);
    if(!res) {
        Free(target);
        Free(buf);
        return FALSE;
    }
    MEMCPY(output, buf, sizeof(IsoDirectoryRecord));
    DUMP_MEMORY(output, sizeof(IsoDirectoryRecord));
    Free(buf);
    HLT;
    if(STRLEN(target) != 0) {
        DUMP_MEMORY(buf, ATAPI_SECTOR_SIZE);
        HLT;
    }
    Free(target);
    Free(buf);
    return TRUE;
}

BOOLEAN ISO9660_READ_DIRECTORY_RECORD(
    CHAR *path,
    IsoDirectoryRecord *root_record,
    IsoDirectoryRecord *out_record
) {
    if (!path || !root_record || !out_record) return FALSE;

    U32 LBA = root_record->extentLocationLE_LBA;
    U32 size = root_record->extentLengthLE;
    if (size == 0 || LBA == 0) return FALSE;

    U8 *modifiable_path = MAlloc(STRLEN(path) + 1);
    if (!modifiable_path) return FALSE;
    STRCPY(modifiable_path, path);

    U8 *slash = (U8 *)STRCHR(modifiable_path, '/');
    if (slash) *slash = '\0';

    *out_record = *root_record;

    BOOLEAN res = ISO9660_READ_FROM_DIRECTORY(
        LBA,
        size,
        (CHAR *)modifiable_path,
        path,
        out_record
    );

    Free(modifiable_path);
    return res;
}

IsoDirectoryRecord *ISO9660_FILERECORD_TO_MEMORY(CHAR *path) {
    CHAR *normPath = ISO9660_NORMALIZE_PATH(path);
    if (!normPath) return NULLPTR;

    PrimaryVolumeDescriptor *_pvd = pvd ? pvd : MAlloc(sizeof(PrimaryVolumeDescriptor));
    if (!_pvd) { Free(normPath); return NULLPTR; }
    
    if (!pvd) {
        if (!ISO9660_READ_PVD(_pvd, sizeof(PrimaryVolumeDescriptor))) {
            Free(normPath);
            Free(_pvd);
            return NULLPTR;
        }
        SetGlobalPVD(_pvd);
    }
    
    if (!ISO9660_IMAGE_CHECK(_pvd)) {
        Free(normPath);
        if (!pvd) Free(_pvd);
        return NULLPTR;
    }


    IsoDirectoryRecord root; // root record
    IsoDirectoryRecord out; // output record
    ISO9660_EXTRACT_ROOT_FROM_PVD(_pvd, &root);

    if (!ISO9660_READ_DIRECTORY_RECORD(normPath, &root, &out)) {
        Free(normPath);
        if (!pvd) Free(_pvd);
        return NULLPTR;
    }
 
    Free(normPath);
    
    IsoDirectoryRecord *retval = MAlloc(sizeof(IsoDirectoryRecord));
    if (!retval) {
        if (!pvd) Free(_pvd);
        return NULLPTR;
    }

    MEMCPY(retval, &out, sizeof(IsoDirectoryRecord));
    return retval;
}

VOIDPTR ISO9660_READ_FILEDATA_TO_MEMORY(IsoDirectoryRecord *fileptr) {
    if (!fileptr) return NULLPTR;
    U32 size = fileptr->extentLengthLE;
    U32 lba = fileptr->extentLocationLE_LBA;
    if (size == 0 || lba == 0) return NULLPTR;

    U8 *buffer = MAlloc(size);
    if (!buffer) return NULLPTR;

    U32 sectors = ISO9660_CALCULATE_SECTORS(size);
    U32 atapiStatus = INITIALIZE_ATAPI();
    if (atapiStatus == ATA_FAILED) { Free(buffer); return NULLPTR; }

    if (READ_CDROM(atapiStatus, lba, sectors, buffer) == ATA_FAILED) {
        Free(buffer);
        return NULLPTR;
    }
    return (VOIDPTR)buffer;
}

void ISO9660_FREE_MEMORY(VOIDPTR ptr) {
    if (!ptr) return;
    Free(ptr);
}

U32 ISO9660_GET_TYPE(IsoDirectoryRecord *rec) {
    if(rec->fileFlags & ISO9660_FILE_FLAG_DIRECTORY) return ISO9660_TYPE_DIRECTORY;
    else return ISO9660_TYPE_FILE;
}
U8 *ISO9660_GET_DIR_CONTENTS(IsoDirectoryRecord *dir) {
    if (!dir) return NULLPTR;

    U32 lba = dir->extentLocationLE_LBA;
    U32 size = dir->extentLengthLE;
    if (lba == 0 || size == 0) return NULLPTR;

    U32 sectors = ISO9660_CALCULATE_SECTORS(size);
    U8 *buffer = MAlloc(sectors * ISO9660_SECTOR_SIZE);
    if (!buffer) return NULLPTR;

    if (READ_CDROM(GET_ATAPI_INFO(), lba, sectors, buffer) == ATA_FAILED) {
        Free(buffer);
        return NULLPTR;
    }

    /* First pass: count entries and total name bytes */
    U32 offset = 0;
    U32 count = 0;
    U32 total_name_bytes = 0;

    while (offset < size) {
        IsoDirectoryRecord *record = (IsoDirectoryRecord *)(buffer + offset);
        if (record->length == 0) break;

        U8 fn_len = record->fileNameLength;
        if (fn_len > 0) {
            // Skip special entries for current (0x00) and parent (0x01)
            if (!(fn_len == 1 && (record->fileIdentifier[0] == 0x00 || record->fileIdentifier[0] == 0x01))) {
                // Ensure we don't exceed reasonable limits
                if (fn_len >= ISO9660_MAX_PATH) { Free(buffer); return NULLPTR; }
                count++;
                total_name_bytes += (U32)fn_len + 1; // include null terminator
                if (count > ISO9660_MAX_DIR_RECORDS) break; // safety cap
            }
        }

        offset += record->length;
    }

    if (count == 0) {
        Free(buffer);
        // Return buffer containing zero count only
        U8 *ret = MAlloc(sizeof(U32));
        if (!ret) return NULLPTR;
        U32 zero = 0;
        MEMCPY(ret, &zero, sizeof(U32));
        return ret;
    }

    /* Allocate result: 4 bytes for count + total_name_bytes */
    U32 result_size = sizeof(U32) + total_name_bytes;
    U8 *result = MAlloc(result_size);
    if (!result) { Free(buffer); return NULLPTR; }

    /* Write count (little-endian) */
    MEMCPY(result, &count, sizeof(U32));

    /* Second pass: copy names after the count */
    U32 write_off = sizeof(U32);
    offset = 0;
    U32 got = 0;

    while (offset < size && got < count) {
        IsoDirectoryRecord *record = (IsoDirectoryRecord *)(buffer + offset);
        if (record->length == 0) break;

        U8 fn_len = record->fileNameLength;
        if (fn_len > 0) {
            if (!(fn_len == 1 && (record->fileIdentifier[0] == 0x00 || record->fileIdentifier[0] == 0x01))) {
                // copy the name (not necessarily null-terminated in source)
                if (write_off + fn_len + 1 > result_size) break; // safety
                STRNCPY((CHAR *)(result + write_off), (CHAR *)record->fileIdentifier, fn_len);
                ((CHAR *)(result + write_off))[fn_len] = '\0';
                write_off += fn_len + 1;
                got++;
            }
        }

        offset += record->length;
    }

    Free(buffer);
    return result;
}
