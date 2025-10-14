#include <DRIVERS/ATA_PIO/ATA_PIO.h>
#include <FS/FAT/FAT.h>
#include <STD/MEM.h>
#include <HEAP/KHEAP.h>
#include <STD/STRING.h>

static BPB bpb ATTRIB_DATA = { 0 };
static U8 bpb_type ATTRIB_DATA = 0;

#define SET_BPB(x, val, sz) MEMCPY(&x, val, sz)

/// @brief Calculate total sectors in the volume
U32 FAT_CalcTotalSectors(BPB *bpb) {
    if (!bpb) return 0;
    if (bpb->TOTAL_SECTORS != 0) return bpb->TOTAL_SECTORS;
    return bpb->LARGE_SECTOR_COUNT;
}

/// @brief Calculate the size of one FAT table in sectors
U32 FAT_CalcFATSize(BPB *bpb) {
    if (!bpb) return 0;
    if (bpb_type == FAT32) {
        return bpb->EXBR.FAT32.SECTORS_PER_FAT;
    } else {
        return bpb->NUM_OF_SECTORS_PER_FAT;
    }
}

/// @brief Calculate the number of sectors occupied by the root directory
U32 FAT_CalcRootDirSectors(BPB *bpb) {
    if (!bpb) return 0;
    if (bpb_type == FAT32) return 0; // root dir is in data clusters
    U32 root_entries = bpb->NUM_OF_ROOT_DIR_ENTRIES;
    U32 bytes_per_sector = bpb->BYTES_PER_SECTOR;
    return (root_entries * 32 + (bytes_per_sector - 1)) / bytes_per_sector;
}

/// @brief Calculate the first data sector
U32 FAT_CalcFirstDataSector(BPB *bpb) {
    if (!bpb) return 0;
    U32 fat_size = FAT_CalcFATSize(bpb);
    U32 root_sectors = FAT_CalcRootDirSectors(bpb);
    return bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * fat_size) + root_sectors;
}

/// @brief Calculate the first FAT sector
U32 FAT_CalcFirstFATSector(BPB *bpb) {
    if (!bpb) return 0;
    return bpb->RESERVED_SECTORS;
}

/// @brief Calculate total data sectors
U32 FAT_CalcDataSectors(BPB *bpb) {
    U32 total_sectors = FAT_CalcTotalSectors(bpb);
    U32 fat_size = FAT_CalcFATSize(bpb);
    U32 root_sectors = FAT_CalcRootDirSectors(bpb);
    return total_sectors - (bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * fat_size) + root_sectors);
}

/// @brief Calculate total clusters
U32 FAT_CalcTotalClusters(BPB *bpb) {
    U32 data_sectors = FAT_CalcDataSectors(bpb);
    return data_sectors / bpb->SECTORS_PER_CLUSTER;
}

/// @brief Determine FAT type (FAT12, FAT16, FAT32)
FAT_TYPE FAT_DetectType(BPB *bpb) {
    if (!bpb) return FAT32; // fallback

    U32 total_clusters = FAT_CalcTotalClusters(bpb);

    if (total_clusters < 4085) return FAT12;
    else if (total_clusters < 65525) return FAT16;
    else return FAT32;
}

U32 cluster_to_sector(U32 cluster) {
    BPB *bpb = GET_BPB();

    if (bpb_type == FAT32) {
        // FAT32: first data sector = reserved + FAT area
        U32 first_data_sector = bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * bpb->EXBR.FAT32.SECTORS_PER_FAT);
        return first_data_sector + ((cluster - 2) * bpb->SECTORS_PER_CLUSTER);
    } 
    else { 
        // FAT12/16: first data sector = reserved + FAT area + root directory
        U32 root_dir_sectors = (bpb->NUM_OF_ROOT_DIR_ENTRIES * 32 + bpb->BYTES_PER_SECTOR - 1) / bpb->BYTES_PER_SECTOR;
        U32 first_data_sector = bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * bpb->NUM_OF_SECTORS_PER_FAT) + root_dir_sectors;

        // cluster = 0 means root directory
        if (cluster == 0) return first_data_sector - root_dir_sectors; // root directory starts here
        return first_data_sector + ((cluster - 2) * bpb->SECTORS_PER_CLUSTER);
    }
}


BOOLEAN LOAD_BPB() { 
    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    if(!ATA_PIO_READ_SECTORS(0, 1, buf)) {
        KFREE(buf);
        return FALSE;
    }
    MEMCPY(&bpb, buf, sizeof(BPB));
    KFREE(buf);
    GET_BPB_TYPE();
    return TRUE;
}
BPB *GET_BPB() { return &bpb; }

U32 GET_BPB_TYPE() {
    bpb_type = FAT_DetectType(&bpb);
    return bpb_type;
}

VOID POPULATE_FAT32_SPECIFIC() {
    bpb.BYTES_PER_SECTOR = 512;
    bpb.SECTORS_PER_CLUSTER = 8;     // 4 KB clusters
    bpb.RESERVED_SECTORS = 32;
    bpb.NUM_OF_FAT = 2;
    bpb.NUM_OF_ROOT_DIR_ENTRIES = 0; // FAT32 stores root dir in cluster chain
    bpb.TOTAL_SECTORS = 0;           // set 0 if >65535 sectors (use LARGE_SECTOR_COUNT)
    bpb.MEDIA_DESCR_TYPE = 0xF8;
    bpb.NUM_OF_SECTORS_PER_FAT = 0;  // not used for FAT32
    bpb.NUM_OF_SECTORS_PER_TRACK = 63;
    bpb.NUM_OF_HEADS = 255;
    bpb.NUM_OF_HIDDEN_SECTORS = 0;
    bpb.LARGE_SECTOR_COUNT = 65536 * 8; // example: ~256MB

    // Extended section (FAT32)
    bpb.EXBR.FAT32.SECTORS_PER_FAT = 8192;
    bpb.EXBR.FAT32.EXT_FLAGS = 0;
    bpb.EXBR.FAT32.FS_VERSION = 0;
    bpb.EXBR.FAT32.ROOT_CLUSTER = 2; // root starts at cluster 2
    bpb.EXBR.FAT32.FS_INFO_SECTOR = 1;
    bpb.EXBR.FAT32.BACKUP_BOOT_SECTOR = 6;
    MEMSET(bpb.EXBR.FAT32.RESERVED, 0, 12);
    bpb.EXBR.FAT32.DRIVE_NUMBER = 0x80;
    bpb.EXBR.FAT32.RESERVED1 = 0;
    bpb.EXBR.FAT32.BOOT_SIGNATURE = 0x29;
    bpb.EXBR.FAT32.VOLUME_ID = 0x12345678;
    SET_BPB(bpb.EXBR.FAT32.VOLUME_LABEL, "NO NAME    ", 11);
    SET_BPB(bpb.EXBR.FAT32.FILE_SYSTEM_TYPE, "FAT32   ", 8);
}

VOID POPULATE_FAT16_SPECIFIC() {
    bpb.BYTES_PER_SECTOR = 512;
    bpb.SECTORS_PER_CLUSTER = 4;     // 2 KB clusters
    bpb.RESERVED_SECTORS = 1;
    bpb.NUM_OF_FAT = 2;
    bpb.NUM_OF_ROOT_DIR_ENTRIES = 512;
    bpb.TOTAL_SECTORS = 65535;
    bpb.MEDIA_DESCR_TYPE = 0xF8;
    bpb.NUM_OF_SECTORS_PER_FAT = 250; // fits small disks
    bpb.NUM_OF_SECTORS_PER_TRACK = 63;
    bpb.NUM_OF_HEADS = 255;
    bpb.NUM_OF_HIDDEN_SECTORS = 0;
    bpb.LARGE_SECTOR_COUNT = 0;

    bpb.EXBR.FAT16.DRIVE_NUMBER = 0x80;
    bpb.EXBR.FAT16.RESERVED = 0;
    bpb.EXBR.FAT16.BOOT_SIGNATURE = 0x29;
    bpb.EXBR.FAT16.VOLUME_ID = 0x87654321;
    SET_BPB(bpb.EXBR.FAT16.VOLUME_LABEL, "NO NAME    ", 11);
    SET_BPB(bpb.EXBR.FAT16.FILE_SYSTEM_TYPE, "FAT16   ", 8);
}

VOID POPULATE_FAT12_SPECIFIC() {
    bpb.BYTES_PER_SECTOR = 512;
    bpb.SECTORS_PER_CLUSTER = 1;     // 512-byte clusters
    bpb.RESERVED_SECTORS = 1;
    bpb.NUM_OF_FAT = 2;
    bpb.NUM_OF_ROOT_DIR_ENTRIES = 224;
    bpb.TOTAL_SECTORS = 2880;        // fits 1.44MB floppy
    bpb.MEDIA_DESCR_TYPE = 0xF0;
    bpb.NUM_OF_SECTORS_PER_FAT = 9;
    bpb.NUM_OF_SECTORS_PER_TRACK = 18;
    bpb.NUM_OF_HEADS = 2;
    bpb.NUM_OF_HIDDEN_SECTORS = 0;
    bpb.LARGE_SECTOR_COUNT = 0;

    bpb.EXBR.FAT12.DRIVE_NUMBER = 0x00;
    bpb.EXBR.FAT12.RESERVED = 0;
    bpb.EXBR.FAT12.BOOT_SIGNATURE = 0x29;
    bpb.EXBR.FAT12.VOLUME_ID = 0xAABBCCDD;
    SET_BPB(bpb.EXBR.FAT12.VOLUME_LABEL, "FLOPPY     ", 11);
    SET_BPB(bpb.EXBR.FAT12.FILE_SYSTEM_TYPE, "FAT12   ", 8);
}

BOOLEAN WRITE_DISK_BPB(FAT_TYPE TYPE) {
    MEMZERO(&bpb, sizeof(BPB));
    bpb.JMPSHORT[0] = 0xEB;
    bpb.JMPSHORT[1] = 0x3C;
    bpb.JMPSHORT[2] = 0x90;
    SET_BPB(bpb.OEM_IDENT, "MSWIN4.1", 8);

    switch (TYPE) {
        case FAT16:
            bpb_type = FAT16;
            POPULATE_FAT16_SPECIFIC();
            break;
        case FAT12:
            bpb_type = FAT12;
            POPULATE_FAT12_SPECIFIC();
            break;
        case FAT32:
        default:
            bpb_type = FAT32;
            POPULATE_FAT32_SPECIFIC();
            break;
    }

    U32 seccount = ATA_CALC_SEC_COUNT(sizeof(BPB));
    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    MEMZERO(buf, ATA_PIO_SECTOR_SIZE);
    MEMCPY(buf, GET_BPB(), sizeof(BPB));
    U32 res = ATA_PIO_WRITE_SECTORS(0, seccount, buf);
    KFREE(buf);
    return res;
}

BOOLEAN POPULATE_BOOTLOADER(VOIDPTR BOOTLOADER_BIN, U32 sz) {
    if (sz + sizeof(BPB) > ATA_PIO_SECTOR_SIZE)
        return FALSE;

    U8 *loader = (U8 *)BOOTLOADER_BIN;

    // if (loader[sz - 2] != 0x55 || loader[sz - 1] != 0xAA)
    //     return FALSE;

    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if (!buf) return FALSE;
    MEMZERO(buf, ATA_PIO_SECTOR_SIZE);

    if (!ATA_PIO_READ_SECTORS(0, 1, buf)) {
        KFREE(buf);
        return FALSE;
    }

    MEMCPY(buf, &bpb, sizeof(BPB));

    MEMCPY(buf + sizeof(BPB), loader, sz);

    buf[510] = 0x55;
    buf[511] = 0xAA;

    BOOLEAN ok = ATA_PIO_WRITE_SECTORS(0, 1, buf);
    KFREE(buf);
    return ok;
}

void FAT_UpdateEntryTime(DIR_ENTRY *entry, U16 date, U16 time) {
    if (!entry) return;
    entry->LAST_MOD_DATE = date;
    entry->LAST_MOD_TIME = time;
}
void FAT_SetCurrentDateTime(U16 *out_date, U16 *out_time) {
    // Example: static for testing, in real OS get RTC values
    *out_date = SET_DATE(2025, 10, 14);  // YYYY, MM, DD
    *out_time = SET_TIME(12, 0, 0);      // HH, MM, SS
}
BOOLEAN FAT_NameExistsInDirectory(U32 dir_cluster, const CHAR *name) {
    if (!name) return FALSE;

    FAT_ENTRY_INFO entries[128];
    U32 count = FAT_ListDirectory(dir_cluster, entries, 128);

    for (U32 i = 0; i < count; i++) {
        // Compare both long name and short name
        if (STRCMP(name, entries[i].long_name) == 0) return TRUE;

        CHAR short_name[12] = {0};
        MEMCPY(short_name, entries[i].short_name, 11);
        if (STRCMP(name, short_name) == 0) return TRUE;
    }

    return FALSE;
}


U8 LFN_Checksum(const U8 short_name[11]) {
    U8 sum = 0;
    for (int i = 0; i < 11; i++)
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    return sum;
}

void FAT_GenerateShortName(const CHAR *long_name, U8 out_short_name[11]) {
    if (!long_name || !out_short_name) return;

    // Clear output buffer and fill with spaces
    for (int i = 0; i < 11; i++) out_short_name[i] = ' ';

    const CHAR *dot = STRRCHR(long_name, '.'); // find extension
    U32 name_len = dot ? (U32)(dot - long_name) : STRLEN(long_name);
    U32 ext_len  = dot ? STRLEN(dot + 1) : 0;

    // Copy filename (first 8 chars)
    for (U32 i = 0; i < name_len && i < 8; i++) {
        CHAR c = long_name[i];
        if (c >= 'a' && c <= 'z') c -= 32; // to uppercase
        out_short_name[i] = c;
    }

    // Copy extension (first 3 chars)
    for (U32 i = 0; i < ext_len && i < 3; i++) {
        CHAR c = dot[1 + i];
        if (c >= 'a' && c <= 'z') c -= 32; // to uppercase
        out_short_name[8 + i] = c;
    }
}

BOOLEAN FAT_WriteLFNEntries(U8 *sector, U32 index, const CHAR *long_name, const U8 short_name[11]) {
    int len = strlen(long_name);
    int entries = (len + 12) / 13; // 13 UTF-16 chars per entry
    U8 checksum = LFN_Checksum(short_name);

    for (int i = 0; i < entries; i++) {
        LFN *lfn = (LFN*)&sector[index + i * 32];
        lfn->ORDER = entries - i;
        if (i == 0) lfn->ORDER |= 0x40; // last LFN entry
        lfn->ATTR = 0x0F;
        lfn->TYPE = 0;
        lfn->CHECKSUM = checksum;
        lfn->FIRST_CLUSTER = 0;

        // fill NAME_1, NAME_2, NAME_3
        int start = i * 13;
        for (int j = 0; j < 5; j++) lfn->NAME_1[j] = (start + j < len) ? long_name[start + j] : 0xFFFF;
        for (int j = 0; j < 6; j++) lfn->NAME_2[j] = (start + 5 + j < len) ? long_name[start + 5 + j] : 0xFFFF;
        for (int j = 0; j < 2; j++) lfn->NAME_3[j] = (start + 11 + j < len) ? long_name[start + 11 + j] : 0xFFFF;
    }

    return TRUE;
}

BOOLEAN FAT32_UpdateFSInfo(U32 free_clusters, U32 next_free_cluster) {
    if (GET_BPB_TYPE() != FAT32) return FALSE;

    FSINFO fsinfo;
    MEMZERO(&fsinfo, sizeof(FSINFO));

    fsinfo.SIGNATURE0 = 0x41615252;
    fsinfo.SIGNATURE1 = 0x61417272;
    fsinfo.FREE_CLUSTER_COUNT = free_clusters;
    fsinfo.CLUSTER_INDICATOR = next_free_cluster;
    fsinfo.TRAIL_SIGNATURE = 0xAA550000; // optional trail signature

    U32 fsinfo_sector = GET_BPB()->EXBR.FAT32.FS_INFO_SECTOR;
    return ATA_PIO_WRITE_SECTORS(fsinfo_sector, 1, &fsinfo);
}

U32 FAT_ReadFATEntry(U32 cluster) {
    BPB *bpb = GET_BPB();
    U32 fat_start = bpb->RESERVED_SECTORS;
    U32 sector, sector_offset;
    U8 buf[512];

    if (bpb_type == FAT32) {
        U32 offset = cluster * 4;
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        return (*(U32*)&buf[sector_offset]) & 0x0FFFFFFF;
    } 
    else if (bpb_type == FAT16) {
        U32 offset = cluster * 2;
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        return *(U16*)&buf[sector_offset];
    } 
    else { // FAT12
        U32 offset = cluster + (cluster / 2);
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        U16 entry = *(U16*)&buf[sector_offset];
        return (cluster & 1) ? (entry >> 4) & 0x0FFF : entry & 0x0FFF;
    }
}

BOOLEAN FAT_WriteFATEntry(U32 cluster, U32 value) {
    BPB *bpb = GET_BPB();
    U32 fat_start = bpb->RESERVED_SECTORS;
    U32 sector, sector_offset;
    U8 buf[512];

    if (bpb_type == FAT32) {
        U32 offset = cluster * 4;
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        U32 *entry = (U32*)&buf[sector_offset];
        *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);
    } 
    else if (bpb_type == FAT16) {
        U32 offset = cluster * 2;
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        *(U16*)&buf[sector_offset] = (U16)value;
    } 
    else { // FAT12
        U32 offset = cluster + (cluster / 2);
        sector = fat_start + (offset / bpb->BYTES_PER_SECTOR);
        sector_offset = offset % bpb->BYTES_PER_SECTOR;
        ATA_PIO_READ_SECTORS(sector, 1, buf);
        U16 entry = *(U16*)&buf[sector_offset];
        if (cluster & 1) {
            entry &= 0x000F;
            entry |= (value << 4) & 0xFFF0;
        } else {
            entry &= 0xF000;
            entry |= value & 0x0FFF;
        }
        *(U16*)&buf[sector_offset] = entry;
    }

    return ATA_PIO_WRITE_SECTORS(sector, 1, buf);
}



U32 SCAN_FREE_FAT_CLUSTERS(U32 clusters_needed) {
    BPB *bpb = GET_BPB();
    U32 total_clusters = (bpb->TOTAL_SECTORS ? bpb->TOTAL_SECTORS : bpb->LARGE_SECTOR_COUNT) /
                         bpb->SECTORS_PER_CLUSTER;

    U32 consecutive_free = 0, first_free = 0;
    for (U32 i = 2; i < total_clusters; i++) {
        U32 fat_entry = FAT_ReadFATEntry(i);
        if (fat_entry == 0) {
            if (consecutive_free == 0) first_free = i;
            consecutive_free++;
            if (consecutive_free >= clusters_needed) return first_free;
        } else {
            consecutive_free = 0;
        }
    }
    return 0;
}

VOID MARK_AND_CHAIN_CLUSTERS(U32 first_cluster, U32 clusters_needed) {
    U32 last_cluster = first_cluster;
    U32 eof_marker;

    if (bpb_type == FAT32) eof_marker = 0x0FFFFFFF;
    else if (bpb_type == FAT16) eof_marker = 0xFFFF;
    else eof_marker = 0x0FFF; // FAT12

    for (U32 i = 1; i < clusters_needed; i++) {
        U32 next_cluster = first_cluster + i;
        FAT_WriteFATEntry(last_cluster, next_cluster);
        last_cluster = next_cluster;
    }
    FAT_WriteFATEntry(last_cluster, eof_marker);
}

BOOLEAN FAT_AllocateFileClusters(U32 size, U32 *out_first_cluster) {
    if (!out_first_cluster) return FALSE;

    BPB *bpb = GET_BPB();
    U32 cluster_size = bpb->BYTES_PER_SECTOR * bpb->SECTORS_PER_CLUSTER;
    U32 clusters_needed = (size + cluster_size - 1) / cluster_size;
    if (clusters_needed == 0) clusters_needed = 1;

    U32 first_cluster = SCAN_FREE_FAT_CLUSTERS(clusters_needed);
    if (first_cluster == 0) return FALSE;

    MARK_AND_CHAIN_CLUSTERS(first_cluster, clusters_needed);

    *out_first_cluster = first_cluster;
    return TRUE;
}


BOOLEAN FAT_WriteFileEntry(U32 dir_cluster,
                           const U8 short_name[11],
                           const CHAR *long_name,
                           U32 start_cluster,
                           U32 size,
                           U8 attr) {
    if (!short_name) return FALSE;

    U32 cluster = dir_cluster;
    U8 sector[512];

    while (1) {
        for (U32 sec = 0; sec < GET_BPB()->SECTORS_PER_CLUSTER; sec++) {
            if (!ATA_PIO_READ_SECTORS(cluster_to_sector(cluster) + sec, 1, sector))
                return FALSE;

            for (U32 i = 0; i < 512; i += 32) {
                if (sector[i] == 0x00 || sector[i] == 0xE5) {
                    // Automatically apply LFN if filename exceeds 11 characters
                    int lfn_entries = (long_name && strlen(long_name) > 11) ? 
                                      (int)((strlen(long_name) + 12) / 13) : 0;

                    if (lfn_entries > 0) {
                        if (!FAT_WriteLFNEntries(sector, i, long_name, short_name))
                            return FALSE;
                    }

                    DIR_ENTRY *entry = (DIR_ENTRY*)&sector[i + lfn_entries * 32];
                    MEMCPY(entry->FILENAME, short_name, 11);
                    entry->ATTRIB = attr;
                    entry->FIRST_CLUSTER_BITS = (start_cluster >> 16) & 0xFFFF;
                    entry->LAST_CLUSTER_BITS  = start_cluster & 0xFFFF;
                    entry->FILE_SIZE = size;

                    // Set creation and modification times
                    U16 date, time;
                    FAT_SetCurrentDateTime(&date, &time);
                    entry->CREATION_DATE = date;
                    entry->CREATION_TIME = time;
                    FAT_UpdateEntryTime(entry, date, time);

                    if (!ATA_PIO_WRITE_SECTORS(cluster_to_sector(cluster) + sec, 1, sector))
                        return FALSE;

                    return TRUE;
                }
            }
        }

        // No free entry in this cluster
        // Could implement cluster chaining for directories if needed
        return FALSE;
    }
}


BOOLEAN FAT_InitDirectoryEntries(U32 dir_cluster, U32 parent_cluster) {
    // "." entry points to dir_cluster itself
    U8 dot_name[11] = {'.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    FAT_WriteFileEntry(dir_cluster, dot_name, ".", dir_cluster, 0, FAT_ATTRB_DIR);

    // ".." entry points to parent_cluster
    U8 dotdot_name[11] = {'.', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    FAT_WriteFileEntry(dir_cluster, dotdot_name, "..", parent_cluster, 0, FAT_ATTRB_DIR);

    return TRUE;
}

BOOLEAN FAT_CreateFile(U32 dir_cluster, const CHAR *filename, U32 size, U32 *out_cluster) {
    if (!filename || !out_cluster) return FALSE;

    // Check for duplicates
    // if (FAT_NameExistsInDirectory(dir_cluster, filename)) return FALSE;

    // Allocate clusters
    U32 first_cluster;
    if (!FAT_AllocateFileClusters(size, &first_cluster)) return FALSE;

    // Generate short name
    U8 short_name[11];
    FAT_GenerateShortName(filename, short_name);

    // Use long name if >11 chars
    const CHAR *long_name = (STRLEN(filename) > 11) ? filename : NULL;

    if (!FAT_WriteFileEntry(dir_cluster, short_name, long_name, first_cluster, size, 0x20)) {
        MARK_AND_CHAIN_CLUSTERS(first_cluster, 1); // rollback
        return FALSE;
    }

    *out_cluster = first_cluster;
    return TRUE;
}


BOOLEAN FAT_CreateDirectory(U32 parent_cluster, const CHAR *dirname, U32 *out_cluster) {
    if (!dirname || !out_cluster) return FALSE;

    // Check for duplicate
    // if (FAT_NameExistsInDirectory(parent_cluster, dirname)) return FALSE;

    // 1. Allocate cluster for the new directory
    U32 new_dir_cluster;
    U32 cluster_size = GET_BPB()->BYTES_PER_SECTOR * GET_BPB()->SECTORS_PER_CLUSTER;

    if (!FAT_AllocateFileClusters(cluster_size, &new_dir_cluster)) {
        return FALSE; // failed to allocate cluster
    }

    // 2. Generate short 8.3 name
    U8 short_name[11];
    FAT_GenerateShortName(dirname, short_name);

    // 3. Write directory entry with optional LFN
    if (!FAT_WriteFileEntry(parent_cluster, short_name, dirname, new_dir_cluster, 0, FAT_ATTRB_DIR)) {
        MARK_AND_CHAIN_CLUSTERS(new_dir_cluster, 1);
        return FALSE;
    }

    // 4. Initialize "." and ".." entries
    if (!FAT_InitDirectoryEntries(new_dir_cluster, parent_cluster)) return FALSE;

    *out_cluster = new_dir_cluster;
    return TRUE;
}


U32 FAT_GetNextCluster(U32 cluster) {
    U32 fat_offset;
    U32 next_cluster;
    U8 sector[512];
    U32 fat_sector;
    U32 ent_offset;

    // FAT32: each entry is 4 bytes
    fat_offset = cluster * 4;
    fat_sector = GET_BPB()->RESERVED_SECTORS + (fat_offset / 512);
    ent_offset = fat_offset % 512;

    // Read FAT sector
    ATA_PIO_READ_SECTORS(fat_sector, 1, sector);

    // Read next cluster value (little-endian)
    next_cluster = *(U32*)&sector[ent_offset] & 0x0FFFFFFF; // mask high 4 bits as FAT32 does

    // Check for end-of-chain
    if (next_cluster >= 0x0FFFFFF8) return 0; // 0 = end of chain

    return next_cluster;
}



U32 FAT_GetRootDirCluster() {
    BPB *bpb = GET_BPB();

    switch (bpb_type) {
        case FAT32:
            return bpb->EXBR.FAT32.ROOT_CLUSTER; // first cluster of root
        case FAT16:
        case FAT12:
        default:
            return 0; // root dir is fixed, cluster 0 indicates root
    }
}

BOOLEAN FAT_CreateRootDirectory() {
    U32 root_cluster = FAT_GetRootDirCluster();
    BPB *bpb = GET_BPB();

    if (bpb_type == FAT32) {
        // FAT32 root directory: allocate first cluster if uninitialized
        if (!root_cluster) {
            U32 cluster;
            U32 cluster_size = bpb->BYTES_PER_SECTOR * bpb->SECTORS_PER_CLUSTER;
            if (!FAT_AllocateFileClusters(cluster_size, &cluster)) return FALSE;
            bpb->EXBR.FAT32.ROOT_CLUSTER = cluster;
            root_cluster = cluster;
        }
    } else {
        // FAT12/16 root directory: clear preallocated sectors
        U32 root_dir_sectors = (bpb->NUM_OF_ROOT_DIR_ENTRIES * 32 + bpb->BYTES_PER_SECTOR - 1) / bpb->BYTES_PER_SECTOR;
        U32 first_sector = bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * (bpb->NUM_OF_SECTORS_PER_FAT));
        U8 zero_buf[512];
        MEMZERO(zero_buf, 512);

        for (U32 s = 0; s < root_dir_sectors; s++) {
            if (!ATA_PIO_WRITE_SECTORS(first_sector + s, 1, zero_buf)) return FALSE;
        }
    }

    // Initialize "." and ".." entries
    return FAT_InitDirectoryEntries(root_cluster, root_cluster);
}

U32 FAT_GetDirFirstSector(U32 cluster) {
    BPB *bpb = GET_BPB();

    if (bpb_type == FAT32) {
        // FAT32: directory cluster chain
        if (cluster == 0) cluster = bpb->EXBR.FAT32.ROOT_CLUSTER; // root cluster
        return ((cluster - 2) * bpb->SECTORS_PER_CLUSTER) + 
               bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * bpb->EXBR.FAT32.SECTORS_PER_FAT);
    } 
    else { 
        // FAT12/16: root directory is fixed
        if (cluster == 0) {
            return bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * bpb->NUM_OF_SECTORS_PER_FAT);
        }
        // Subdirectory: calculate sector from cluster chain
        return ((cluster - 2) * bpb->SECTORS_PER_CLUSTER) + 
               bpb->RESERVED_SECTORS + (bpb->NUM_OF_FAT * bpb->NUM_OF_SECTORS_PER_FAT) +
               ((bpb->NUM_OF_ROOT_DIR_ENTRIES * 32 + bpb->BYTES_PER_SECTOR - 1) / bpb->BYTES_PER_SECTOR);
    }
}

BOOLEAN FAT_ReadFileData(U32 first_cluster, U8 *data, U32 size) {
    if (!data || size == 0) return FALSE;

    BPB *bpb = GET_BPB();
    U32 cluster = first_cluster;
    U32 cluster_size = bpb->BYTES_PER_SECTOR * bpb->SECTORS_PER_CLUSTER;
    U32 bytes_read = 0;

    while (cluster >= 2 && bytes_read < size) {
        U32 sector_start = cluster_to_sector(cluster);
        U8 sector_buf[512];

        for (U32 sec = 0; sec < bpb->SECTORS_PER_CLUSTER && bytes_read < size; sec++) {
            if (!ATA_PIO_READ_SECTORS(sector_start + sec, 1, sector_buf)) return FALSE;

            U32 bytes_to_copy = (size - bytes_read) > bpb->BYTES_PER_SECTOR ? bpb->BYTES_PER_SECTOR : (size - bytes_read);
            MEMCPY(data + bytes_read, sector_buf, bytes_to_copy);

            bytes_read += bytes_to_copy;
        }

        // Get next cluster from FAT
        cluster = FAT_ReadFATEntry(cluster);
        if (bpb_type == FAT32) cluster &= 0x0FFFFFFF;

        // Stop at EOF marker
        if ((bpb_type == FAT32 && cluster >= 0x0FFFFFF8) ||
            (bpb_type == FAT16 && cluster >= 0xFFF8) ||
            (bpb_type == FAT12 && cluster >= 0x0FF8)) break;
    }

    return TRUE;
}

BOOLEAN FAT_WriteFileData(U32 first_cluster, const U8 *data, U32 size) {
    if (!data || size == 0) return FALSE;

    BPB *bpb = GET_BPB();
    U32 cluster = first_cluster;
    U32 cluster_size = bpb->BYTES_PER_SECTOR * bpb->SECTORS_PER_CLUSTER;
    U32 bytes_written = 0;

    while (cluster >= 2 && bytes_written < size) {
        U32 sector_start = cluster_to_sector(cluster);
        U8 sector_buf[512];

        for (U32 sec = 0; sec < bpb->SECTORS_PER_CLUSTER && bytes_written < size; sec++) {
            U32 bytes_to_write = (size - bytes_written) > bpb->BYTES_PER_SECTOR ? bpb->BYTES_PER_SECTOR : (size - bytes_written);
            MEMZERO(sector_buf, 512);                       // clear remaining bytes
            MEMCPY(sector_buf, data + bytes_written, bytes_to_write);

            if (!ATA_PIO_WRITE_SECTORS(sector_start + sec, 1, sector_buf)) return FALSE;

            bytes_written += bytes_to_write;
        }

        // Get next cluster from FAT
        cluster = FAT_ReadFATEntry(cluster);
        // For FAT32: mask upper 4 bits
        if (bpb_type == FAT32) cluster &= 0x0FFFFFFF;

        // Stop at EOF marker
        if ((bpb_type == FAT32 && cluster >= 0x0FFFFFF8) ||
            (bpb_type == FAT16 && cluster >= 0xFFF8) ||
            (bpb_type == FAT12 && cluster >= 0x0FF8)) break;
    }

    return TRUE;
}


static void split_path(const CHAR *path, CHAR components[][FAT_MAX_FILENAME], U32 *count) {
    *count = 0;
    const CHAR *start = path;
    CHAR buf[FAT_MAX_FILENAME];
    U32 idx = 0;

    while (*start) {
        if (*start == '/' || *start == '\\') {
            if (idx > 0) {
                buf[idx] = 0;
                STRCPY(components[*count], buf);
                (*count)++;
                idx = 0;
            }
        } else if (idx < FAT_MAX_FILENAME-1) {
            buf[idx++] = *start;
        }
        start++;
    }
    if (idx > 0) { 
        buf[idx] = 0;
        STRCPY(components[*count], buf);
        (*count)++;
    }
}

BOOLEAN FAT_CreateFilePath(const CHAR *path, U8 flags) {
    if (!path || path[0] != '/') return FALSE;

    CHAR components[16][FAT_MAX_FILENAME]; // max 16 nested directories
    U32 count = 0;
    split_path(path, components, &count);
    if (count == 0) return FALSE;

    U32 cluster = FAT_GetRootDirCluster();

    // Traverse/create parent directories
    for (U32 i = 0; i < count - 1; i++) {
        U32 next_cluster;

        // Check if directory exists
        FAT_ENTRY_INFO dir_info;
        if (FAT_NameExistsInDirectory(cluster, components[i])) {
            CHAR full_path[FAT_MAX_PATH];
            STRCPY(full_path, "/");
            for (U32 j = 0; j <= i; j++) {
                STRCAT(full_path, components[j]);
                if (j != i) STRCAT(full_path, "/");
            }
            if (!FAT_GetEntryByPath(full_path, &dir_info)) return FALSE;
            next_cluster = dir_info.first_cluster;
        } else {
            // Create directory
            if (!FAT_CreateDirectory(cluster, components[i], &next_cluster)) return FALSE;
        }

        cluster = next_cluster;
    }

    // Check for duplicate file
    if (FAT_NameExistsInDirectory(cluster, components[count - 1])) return FALSE;

    // Create file
    U32 file_cluster;
    return FAT_CreateFile(cluster, components[count - 1], 0, &file_cluster);
}

BOOLEAN FAT_CreateDirectoryPath(const CHAR *path, U8 flags) {
    if (!path || path[0] != '/') return FALSE;

    CHAR components[MAX_NESTED_DIRS][FAT_MAX_FILENAME];
    U32 count = 0;
    split_path(path, components, &count);

    U32 cluster = FAT_GetRootDirCluster();

    for (U32 i = 0; i < count; i++) {
        U32 next_cluster;

        // Check if directory already exists
        if (FAT_NameExistsInDirectory(cluster, components[i])) {
            CHAR full_path[FAT_MAX_PATH];
            STRCPY(full_path, "/");
            for (U32 j = 0; j <= i; j++) {
                STRCAT(full_path, components[j]);
                if (j != i) STRCAT(full_path, "/");
            }
            FAT_ENTRY_INFO dir_info;
            if (!FAT_GetEntryByPath(full_path, &dir_info)) return FALSE;
            next_cluster = dir_info.first_cluster;
        } else {
            // Create directory
            if (!FAT_CreateDirectory(cluster, components[i], &next_cluster)) return FALSE;
        }

        cluster = next_cluster;
    }

    return TRUE;
}

BOOLEAN FAT_CreateFileWithContents(const CHAR *path, const U8 *data, U32 size, U8 flags) {
    if (!path || path[0] != '/') return FALSE;

    CHAR components[MAX_NESTED_DIRS][FAT_MAX_FILENAME];
    U32 count = 0;
    split_path(path, components, &count);
    if (count == 0) return FALSE;

    // Traverse directories to parent dir
    U32 cluster = FAT_GetRootDirCluster();
    for (U32 i = 0; i < count-1; i++) {
        U32 next_cluster;
        if (!FAT_CreateDirectory(cluster, components[i], &next_cluster)) return FALSE;
        cluster = next_cluster;
    }

    // Create file
    U32 file_cluster;
    if (!FAT_CreateFile(cluster, components[count-1], size, &file_cluster)) return FALSE;

    // Write contents if any
    if (data && size > 0) {
        if (!FAT_WriteFileData(file_cluster, data, size)) return FALSE;
    }

    return TRUE;
}

BOOLEAN FAT_CreateFileFull(const CHAR *path, const U8 *data, U32 size, U8 flags) {
    if (data && size > 0) {
        return FAT_CreateFileWithContents(path, data, size, flags);
    } else {
        return FAT_CreateFilePath(path, flags);
    }
}

void FAT_ParseTime(U16 fat_time, U8 *hour, U8 *min, U8 *sec) {
    *hour = (fat_time >> 11) & 0x1F;
    *min  = (fat_time >> 5)  & 0x3F;
    *sec  = (fat_time & 0x1F) * 2;
}

void FAT_ParseDate(U16 fat_date, U16 *year, U8 *month, U8 *day) {
    *year  = ((fat_date >> 9) & 0x7F) + 1980;
    *month = (fat_date >> 5) & 0x0F;
    *day   = fat_date & 0x1F;
}

BOOLEAN FAT_GetEntryByPath(CHAR *path, FAT_ENTRY_INFO *out_info) {
    if (!path || !out_info) return FALSE;

    CHAR components[16][FAT_MAX_FILENAME];
    U32 count = 0;
    split_path(path, components, &count);
    if (count == 0) return FALSE;

    U32 cluster = FAT_GetRootDirCluster();

    for (U32 i = 0; i < count; i++) {
        // read cluster contents
        U32 found = 0;
        U8 sector[512];

        for (U32 sec = 0; sec < GET_BPB()->SECTORS_PER_CLUSTER; sec++) {
            ATA_PIO_READ_SECTORS(cluster_to_sector(cluster) + sec, 1, sector);
            for (U32 offset = 0; offset < 512; offset += 32) {
                DIR_ENTRY *entry = (DIR_ENTRY*)&sector[offset];

                if (entry->FILENAME[0] == 0x00) break; // end of entries

                // check for LFN (attribute 0x0F)
                if (entry->ATTRIB == FAT_ATTRIB_LFN) continue;

                CHAR short_name[12] = {0};
                MEMCPY(short_name, entry->FILENAME, 11);

                if (STRCMP(components[i], short_name) == 0 || STRCMP(components[i], "") == 0) {
                    if (i == count-1) {
                        // last component = file/dir found
                        MEMCPY(out_info->short_name, entry->FILENAME, 11);
                        out_info->first_cluster = ((U32)entry->FIRST_CLUSTER_BITS << 16) | entry->LAST_CLUSTER_BITS;
                        out_info->size = entry->FILE_SIZE;
                        out_info->attributes = entry->ATTRIB;
                        out_info->creation_date = entry->CREATION_DATE;
                        out_info->creation_time = entry->CREATION_TIME;
                        out_info->last_mod_date = entry->LAST_MOD_DATE;
                        out_info->last_mod_time = entry->LAST_MOD_TIME;
                        out_info->last_access_date = entry->LAST_ACCESS_DATE;
                        // TODO: LFN decoding
                        out_info->lfn_used = 0; // simplified for now
                        STRCPY(out_info->long_name, components[i]);
                        return TRUE;
                    } else {
                        // next component = subdirectory
                        cluster = ((U32)entry->FIRST_CLUSTER_BITS << 16) | entry->LAST_CLUSTER_BITS;
                        found = 1;
                        break;
                    }
                }
            }
            if (found) break;
        }

        if (!found) return FALSE; // path component not found
    }

    return FALSE;
}

BOOLEAN FAT_GetFileContents(FAT_ENTRY_INFO *entry, U8 *buffer, U32 buffer_size) {
    if (!entry || !buffer) return FALSE;

    return FAT_ReadFileData(entry->first_cluster, buffer, entry->size); 
}
U32 FAT_ListDirectory(U32 dir_cluster, FAT_ENTRY_INFO *entries, U32 max_entries) {
    U32 count = 0;
    U8 sector[512];
    CHAR lfn_buffer[256];

    while (dir_cluster >= 2 && count < max_entries) {
        for (U32 sec = 0; sec < GET_BPB()->SECTORS_PER_CLUSTER; sec++) {
            ATA_PIO_READ_SECTORS(cluster_to_sector(dir_cluster) + sec, 1, sector);

            MEMZERO(lfn_buffer, sizeof(lfn_buffer));
            int lfn_last_order = 0;

            for (U32 offset = 0; offset < 512; offset += 32) {
                DIR_ENTRY *entry = (DIR_ENTRY*)&sector[offset];
                if (entry->FILENAME[0] == 0x00) break; // End of entries in this cluster
                if (entry->FILENAME[0] == 0xE5) continue; // Deleted entry

                if (entry->ATTRIB == FAT_ATTRIB_LFN) {
                    LFN *lfn = (LFN*)entry;
                    int order = lfn->ORDER & 0x3F;
                    int pos = (order - 1) * 13;

                    for (int j = 0; j < 5; j++) if (lfn->NAME_1[j] != 0xFFFF) lfn_buffer[pos++] = (CHAR)lfn->NAME_1[j];
                    for (int j = 0; j < 6; j++) if (lfn->NAME_2[j] != 0xFFFF) lfn_buffer[pos++] = (CHAR)lfn->NAME_2[j];
                    for (int j = 0; j < 2; j++) if (lfn->NAME_3[j] != 0xFFFF) lfn_buffer[pos++] = (CHAR)lfn->NAME_3[j];

                    lfn_last_order = order;
                    continue;
                }

                // Normal directory entry
                if (count >= max_entries) return count;
                FAT_ENTRY_INFO *e = &entries[count++];

                MEMCPY(e->short_name, entry->FILENAME, 11);
                e->first_cluster = ((U32)entry->FIRST_CLUSTER_BITS << 16) | entry->LAST_CLUSTER_BITS;
                e->size = entry->FILE_SIZE;
                e->attributes = entry->ATTRIB;
                e->creation_date = entry->CREATION_DATE;
                e->creation_time = entry->CREATION_TIME;
                e->last_mod_date = entry->LAST_MOD_DATE;
                e->last_mod_time = entry->LAST_MOD_TIME;
                e->last_access_date = entry->LAST_ACCESS_DATE;

                if (lfn_last_order > 0) {
                    // terminate the string properly
                    int len = lfn_last_order * 13;
                    if (len > 255) len = 255;
                    lfn_buffer[len] = 0;
                    STRCPY(e->long_name, lfn_buffer);
                    e->lfn_used = 1;
                } else {
                    MEMZERO(e->long_name, sizeof(e->long_name));
                    e->lfn_used = 0;
                }

                MEMZERO(lfn_buffer, sizeof(lfn_buffer));
                lfn_last_order = 0;
            }
        }

        // Move to next cluster in the directory chain
        dir_cluster = FAT_GetNextCluster(dir_cluster);
    }

    return count;
}


