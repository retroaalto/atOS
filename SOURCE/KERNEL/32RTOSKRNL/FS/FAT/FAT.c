#include <DRIVERS/ATA_PIO/ATA_PIO.h>
#include <DRIVERS/CMOS/CMOS.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <FS/FAT/FAT.h>
#include <HEAP/KHEAP.h>
#include <PROC/PROC.h>
#include <STD/MEM.h>
#include <STD/MATH.h>
#include <STD/STRING.h>

static BPB bpb ATTRIB_DATA = { 0 };
static U32 *fat32 ATTRIB_DATA = NULLPTR;
static DIR_ENTRY *root_dir = NULLPTR;
static U32 root_dir_end ATTRIB_DATA = 0;
static BOOL bpb_loaded ATTRIB_DATA = 0;
static FSINFO fsinfo ATTRIB_DATA = { 0 };

#define SET_BPB(x, val, sz) MEMCPY(&x, val, sz)

// Writes a single sector to disk at absolute sector number
BOOL FAT_WRITE_SECTOR_ON_DISK(U32 lba, const U8 *buf) {
    if (!buf) return FALSE;
    // ATA_PIO_WRITE_SECTORS expects LBA, count, buffer
    return ATA_PIO_WRITE_SECTORS(lba, 1, buf);
}

// Reads a single sector from disk at absolute sector number
BOOL FAT_READ_SECTOR_FROM_DISK(U32 lba, U8 *buf) {
    if (!buf) return FALSE;
    return ATA_PIO_READ_SECTORS(lba, 1, buf);
}

// Write cluster buffer (cluster -> disk sector mapping)
BOOL FAT_WRITE_CLUSTER(U32 cluster, const U8 *buf) {
    if (!buf) return FALSE;
    if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U32 data_start_sector = bpb.RESERVED_SECTORS + (bpb.NUM_OF_FAT * bpb.EXBR.SECTORS_PER_FAT);
    U32 sector = data_start_sector + (cluster - FIRST_ALLOWED_CLUSTER_NUMBER) * bpb.SECTORS_PER_CLUSTER;
    
    return ATA_PIO_WRITE_SECTORS(sector, bpb.SECTORS_PER_CLUSTER, buf);
}

// Read cluster buffer (cluster -> disk sector mapping)
BOOL FAT_READ_CLUSTER(U32 cluster, U8 *buf) {
    if (!buf) return FALSE;
    if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U32 data_start_sector = bpb.RESERVED_SECTORS + (bpb.NUM_OF_FAT * bpb.EXBR.SECTORS_PER_FAT);
    U32 sector = data_start_sector + (cluster - FIRST_ALLOWED_CLUSTER_NUMBER) * bpb.SECTORS_PER_CLUSTER;
    
    return ATA_PIO_READ_SECTORS(sector, bpb.SECTORS_PER_CLUSTER, buf);
}


BOOLEAN READ_FAT(){
    return TRUE;
}
BOOLEAN READ_ROOT_DIR(){
    // root_dir_end
    return TRUE;
}

BOOLEAN LOAD_BPB() { 
    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    if(!FAT_READ_SECTOR_FROM_DISK(0, buf)) {
        KFREE(buf);
        return FALSE;
    }
    MEMCPY(&bpb, buf, sizeof(BPB));

    KFREE(buf);
    if(!READ_FAT()) return FALSE;
    if(!READ_ROOT_DIR()) return FALSE;
    bpb_loaded = TRUE;
    return TRUE;
}

BPB *GET_BPB() { 
    if(!bpb_loaded) {
        if(!LOAD_BPB()) return NULLPTR;
    } 
    return &bpb;
}
BOOLEAN GET_BPB_LOADED(){ 
    // TODO: read from disk and check ids
    return TRUE; 
}
BOOLEAN WRITE_DISK_BPB() {
    MEMZERO(&bpb, sizeof(BPB));
    bpb.JMPSHORT[0] = 0xEB;
    bpb.JMPSHORT[1] = 0x3C;
    bpb.JMPSHORT[2] = 0x90;
    SET_BPB(bpb.OEM_IDENT, "MSWIN4.1", 8);

    bpb.BYTES_PER_SECTOR = BYTES_PER_SECT;
    bpb.SECTORS_PER_CLUSTER = SECT_PER_CLUST;     // 4 KB clusters
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
    bpb.EXBR.SECTORS_PER_FAT = 8192;
    bpb.EXBR.EXT_FLAGS = 0;
    bpb.EXBR.FS_VERSION = 0;
    bpb.EXBR.ROOT_CLUSTER = 2; // root starts at cluster 2
    bpb.EXBR.FS_INFO_SECTOR = 1;
    bpb.EXBR.BACKUP_BOOT_SECTOR = 6;
    MEMSET(bpb.EXBR.RESERVED, 0, 12);
    bpb.EXBR.DRIVE_NUMBER = 0x80;
    bpb.EXBR.RESERVED1 = 0;
    bpb.EXBR.BOOT_SIGNATURE = 0x29;
    bpb.EXBR.VOLUME_ID = 0x12345678;
    SET_BPB(bpb.EXBR.VOLUME_LABEL, "ATOSRT__FS_", 11);
    SET_BPB(bpb.EXBR.FILE_SYSTEM_TYPE, "FAT32   ", 8);

    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    MEMZERO(buf, ATA_PIO_SECTOR_SIZE);
    MEMCPY(buf, &bpb, sizeof(BPB));
    U32 res = FAT_WRITE_SECTOR_ON_DISK(0, buf);
    KFREE(buf);
    
    bpb_loaded = TRUE;
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

    if (!FAT_READ_SECTOR_FROM_DISK(0, buf)) {
        KFREE(buf);
        return FALSE;
    }

    MEMCPY(buf, &bpb, sizeof(BPB));

    MEMCPY(buf + sizeof(BPB), loader, sz);

    buf[510] = 0x55;
    buf[511] = 0xAA;

    BOOLEAN ok = FAT_WRITE_SECTOR_ON_DISK(0, buf);
    KFREE(buf);
    return ok;
}

void FAT_UPDATETIMEDATE(DIR_ENTRY *entry) {
    if (!entry) return;
    RTC_DATE_TIME r = GET_SYS_TIME();
    entry->LAST_MOD_DATE = SET_DATE(r.year, r.month, r.day_of_month);
    entry->LAST_MOD_TIME = SET_TIME(r.hours, r.minutes, r.seconds);
}

VOID FAT_CREATION_UPDATETIMEDATE(DIR_ENTRY *entry) {
    RTC_DATE_TIME r = GET_SYS_TIME();
    entry->CREATION_TIME_HUN_SEC = 0;
    entry->CREATION_DATE = SET_DATE(r.year, r.month, r.day_of_month);
    entry->CREATION_TIME = SET_TIME(r.hours, r.minutes, r.seconds);
    entry->LAST_MOD_DATE = SET_DATE(r.year, r.month, r.day_of_month);
    entry->LAST_MOD_TIME = SET_TIME(r.hours, r.minutes, r.seconds);
}

BOOLEAN INITIAL_WRITE_FAT() {
    if (!bpb_loaded) return FALSE;

    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    U32 bytes_per_fat = fat_sectors * bpb.BYTES_PER_SECTOR;

    // Allocate and zero FAT buffer
    U8 *fatbuf = KMALLOC(bytes_per_fat);
    if (!fatbuf) return FALSE;
    MEMZERO(fatbuf, bytes_per_fat);

    fat32 = (U32 *)fatbuf;

    // Mark first 3 FAT entries
    fat32[0] = 0x0FFFFFF8;                  // Media descriptor + reserved
    fat32[1] = 0xFFFFFFFF;                  // Reserved
    fat32[2] = FAT32_END_OF_CHAIN;          // Root directory EOC

    // Total clusters
    U32 total_clusters = bytes_per_fat / 4;

    // Mark remaining clusters as free
    for (U32 i = 3; i < total_clusters; i++) {
        fat32[i] = FAT32_FREE_CLUSTER;      // 0x00000000
    }

    // Write FAT #1
    if (!ATA_PIO_WRITE_SECTORS(bpb.RESERVED_SECTORS, fat_sectors, fatbuf)) {
        KFREE(fatbuf);
        return FALSE;
    }

    // Write FAT #2 (backup)
    if (!ATA_PIO_WRITE_SECTORS(bpb.RESERVED_SECTORS + fat_sectors, fat_sectors, fatbuf)) {
        KFREE(fatbuf);
        return FALSE;
    }

    return TRUE;
}


BOOL FAT_FLUSH(void) {
    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    // reserved sectors points to start of FAT in your driver usage
    return ATA_PIO_WRITE_SECTORS(bpb.RESERVED_SECTORS, fat_sectors, fat32);
}

U32 FIND_NEXT_FREE_CLUSTER() {
    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    U32 bytes_per_fat = fat_sectors * bpb.BYTES_PER_SECTOR;
    U32 total_clusters = bytes_per_fat / 4;

    for(U32 i = 1; i < total_clusters; i++) {
        if(fat32[i] == FAT32_FREE_CLUSTER) {
            return i;
        }
    }
    return 0;
}

U32 FAT32_GetLastCluster(U32 start_cluster) {
    U32 current = start_cluster;
    while (fat32[current] < FAT32_END_OF_CHAIN && fat32[current] != 0) {
        current = fat32[current];
    }
    return current;
}

BOOL FAT32_AppendCluster(U32 start_cluster, U32 new_cluster) {
    if (new_cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U32 last = FAT32_GetLastCluster(start_cluster);
    fat32[last] = new_cluster;          // link old end to new
    fat32[new_cluster] = FAT32_END_OF_CHAIN;     // mark new as end
    return TRUE;
}

U32 BYTES_FREE_IN_CLUSTER(U32 cluster) {
    U8 buf[CLUSTER_SIZE]; // cluster_size = sectors_per_cluster * bytes_per_sector
    FAT_READ_CLUSTER(cluster, buf); // read cluster into buffer

    U32 used_bytes = 0;
    for(U32 offset = 0; offset < CLUSTER_SIZE; offset += 32) {
        DIR_ENTRY *entry = (DIR_ENTRY *)(buf + offset);
        if(entry->FILENAME[0] != 0x00 && entry->FILENAME[0] != 0xE5)
            used_bytes += 32;
        else
            break; // first free slot found, rest are free
    }

    return CLUSTER_SIZE - used_bytes;
}

BOOL MARK_CLUSTER_FREE(U32 cluster) {
    if(cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE; // reserved clusters

    fat32[cluster] = FAT32_FREE_CLUSTER; // mark as free

    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    return ATA_PIO_WRITE_SECTORS(bpb.RESERVED_SECTORS, fat_sectors, fat32);
}

BOOL MARK_CLUSTER_USED(U32 cluster) {
    if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;
    fat32[cluster] = FAT32_END_OF_CHAIN;
    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    return ATA_PIO_WRITE_SECTORS(bpb.RESERVED_SECTORS, fat_sectors, fat32);
}

// --- Find a free slot (cluster+offset) in a directory chain ---
// returns TRUE and sets *out_cluster and *out_offset (byte offset in cluster) when free slot found.
// If no free slot in existing chain, appends a new cluster and returns its start offset 0.
BOOL DIR_FIND_FREE_SLOT(U32 dir_start_cluster, U32 *out_cluster, U32 *out_offset) {
    if (dir_start_cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U32 cluster = dir_start_cluster;
    U8 buf[CLUSTER_SIZE];

    while (1) {
        if (!FAT_READ_CLUSTER(cluster, buf)) return FALSE;

        // scan entries in this cluster
        for (U32 offset = 0; offset < CLUSTER_SIZE; offset += 32) {
            DIR_ENTRY *e = (DIR_ENTRY *)(buf + offset);
            if (e->FILENAME[0] == 0x00 || e->FILENAME[0] == 0xE5) {
                *out_cluster = cluster;
                *out_offset = offset;
                return TRUE;
            }
        }

        // no free slot in this cluster -> check chain
        if (fat32[cluster] >= FAT32_END_OF_CHAIN) {
            // need to allocate and append a new cluster for directory
            U32 new_cluster = FIND_NEXT_FREE_CLUSTER();
            if (new_cluster == 0) return FALSE;
            fat32[cluster] = new_cluster;
            fat32[new_cluster] = FAT32_END_OF_CHAIN;
            if (!FAT_FLUSH()) return FALSE;

            // clear new cluster on disk (zero it)
            MEMZERO(buf, CLUSTER_SIZE);
            if (!FAT_WRITE_CLUSTER(new_cluster, buf)) return FALSE;

            *out_cluster = new_cluster;
            *out_offset = 0;
            return TRUE;
        }

        // follow chain
        cluster = fat32[cluster];
        if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE; // corrupted
    }
}

U8 LFN_CHECKSUM(U8 *shortname) {
    U8 sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + shortname[i];
    }
    return sum;
}

VOID FAT_83FILENAMEFY(DIR_ENTRY *entry, U8 *original) {
    PU8 ext = STRRCHR(original, '.');
    U32 i;

    for(i = 0; i < 3; i++) entry->FILENAME[8 + i] = ' ';

    // Fill extension
    if(ext) {
        ext++; // skip dot
        for(i = 0; i < 3 && ext[i] != '\0'; i++) {
            entry->FILENAME[8 + i] = TOUPPER(ext[i]);
        }
    }

    // Compute base name length
    U32 len = (ext) ? (U32)((ext - 1) - original) : STRLEN(original); // ext - 1 because ext points to after dot
    if(len > 8) len = 8;

    // Fill base name
    for(i = 0; i < len; i++) {
        entry->FILENAME[i] = TOUPPER(original[i]);
    }

    // Fill remaining name bytes with spaces
    for(; i < 8; i++) entry->FILENAME[i] = ' ';
}

VOID FILL_LFNs(LFN *LFNs, U32 n, U8 *name) {
    U32 str_index = 0;
    U32 strlen = STRLEN(name);
    for(U32 i = 0; i < n; i++) {
        LFN *ent = &LFNs[n - 1 - i]; // reverse order
        ent->ALWAYS_ZERO = 0;
        ent->CHECKSUM = LFN_CHECKSUM(name);
        ent->TYPE = FAT_ATTRIB_LFN;
        ent->ORDER = i + 1;
        if(i == n - 1) ent->ORDER |= LFN_LAST_ENTRY;

        for(U32 j = 0; j < 5; j++) ent->NAME_1[j] = 0xFFFF;
        for(U32 j = 0; j < 6; j++) ent->NAME_2[j] = 0xFFFF;
        for(U32 j = 0; j < 2; j++) ent->NAME_3[j] = 0xFFFF;

        for(U32 j = 0; j < 5 && str_index < strlen; j++, str_index++) {
            ent->NAME_1[j] = name[str_index];
        }
        for(U32 j = 0; j < 6 && str_index < strlen; j++, str_index++) {
            ent->NAME_2[j] = name[str_index];
        }
        for(U32 j = 0; j < 2 && str_index < strlen; j++, str_index++) {
            ent->NAME_3[j] = name[str_index];
        }
    }
}

BOOLEAN WRITE_FILEDATA(DIR_ENTRY *out_ent, PU8 filedata, U32 sz) {
    if (!out_ent) return FALSE;
    if (sz == 0) {
        // no data -> start cluster 0 (or leave as-is)
        out_ent->LOW_CLUSTER_BITS = 0;
        out_ent->HIGH_CLUSTER_BITS = 0;
        return TRUE;
    }

    U32 clusters_needed = (sz + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    U32 first_cluster = 0;
    U32 prev_cluster = 0;
    U32 remaining = sz;
    PU8 src = filedata;

    U8 buf[CLUSTER_SIZE];

    for (U32 i = 0; i < clusters_needed; ++i) {
        U32 c = FIND_NEXT_FREE_CLUSTER();
        if (c == 0) {
            // out of clusters -> free any allocated so far
            U32 cur = first_cluster;
            while (cur) {
                U32 next = (fat32[cur] >= FAT32_END_OF_CHAIN) ? 0 : fat32[cur];
                fat32[cur] = FAT32_FREE_CLUSTER;
                cur = next;
            }
            FAT_FLUSH();
            return FALSE;
        }

        // mark used (temporarily link as EOC)
        fat32[c] = FAT32_END_OF_CHAIN;

        if (prev_cluster != 0) {
            fat32[prev_cluster] = c; // link previous to new
        } else {
            first_cluster = c;
        }
        prev_cluster = c;

        // write cluster data
        MEMZERO(buf, CLUSTER_SIZE);
        U32 tocopy = (remaining > CLUSTER_SIZE) ? CLUSTER_SIZE : remaining;
        MEMCPY(buf, src, tocopy);
        if (!FAT_WRITE_CLUSTER(c, buf)) {
            // rollback on failure: free allocated clusters
            U32 cur = first_cluster;
            while (cur) {
                U32 next = (fat32[cur] >= FAT32_END_OF_CHAIN) ? 0 : fat32[cur];
                fat32[cur] = FAT32_FREE_CLUSTER;
                cur = next;
            }
            FAT_FLUSH();
            return FALSE;
        }

        src += tocopy;
        remaining -= tocopy;
    }

    // last cluster already has EOC set in fat32[prev_cluster] = EOC
    if (!FAT_FLUSH()) return FALSE;

    // set start cluster into directory entry
    U32 start = first_cluster;
    out_ent->LOW_CLUSTER_BITS = start & 0xFFFF;
    out_ent->HIGH_CLUSTER_BITS = (start >> 16) & 0xFFFF;

    return TRUE;
}

BOOLEAN CREATE_DIR_ENTRY(U32 parent_cluster, U8 *FILENAME, U8 ATTRIB, PU8 filedata, U32 filedata_size, DIR_ENTRY *out) {
    if (!out || !FILENAME) return FALSE;

    MEMZERO(out, sizeof(DIR_ENTRY));
    out->ATTRIB = ATTRIB;
    FAT_83FILENAMEFY(out, FILENAME); // pass pointer, not &out
    FAT_CREATION_UPDATETIMEDATE(out);
    out->CREATION_TIME_HUN_SEC = (get_ticks() % 199) + 1;

    // prepare LFN entries if needed
    U32 num_of_lfn_entries = 0;
    LFN LFNs[MAX_LFN_COUNT];
    MEMZERO(LFNs, sizeof(LFNs));
    if (STRLEN(FILENAME) > 11) {
        num_of_lfn_entries = (STRLEN(FILENAME) + CHARS_PER_LFN - 1) / CHARS_PER_LFN;
        FILL_LFNs(LFNs, num_of_lfn_entries, FILENAME);
    }

    // if filedata present, allocate clusters and write data (this sets out->cluster fields)
    if (filedata_size > 0 && filedata) {
        out->FILE_SIZE = filedata_size;
        if (!WRITE_FILEDATA(out, filedata, filedata_size)) return FALSE;
    } else {
        out->FILE_SIZE = 0;
        out->LOW_CLUSTER_BITS = 0;
        out->HIGH_CLUSTER_BITS = 0;
    }

    // find free slot in parent directory (this will append cluster if parent full)
    U32 slot_cluster = 0;
    U32 slot_offset = 0;
    if (!DIR_FIND_FREE_SLOT(parent_cluster, &slot_cluster, &slot_offset)) return FALSE;

    // read cluster, write LFN entries + short entry
    U8 buf[CLUSTER_SIZE];
    if (!FAT_READ_CLUSTER(slot_cluster, buf)) return FALSE;

    // verify there is room; if not (edge case), extend cluster and re-read/adjust (DIR_FIND_FREE_SLOT should guarantee 32 bytes)
    U32 available = CLUSTER_SIZE - slot_offset;
    U32 required = (num_of_lfn_entries * sizeof(LFN)) + sizeof(DIR_ENTRY);
    if (required > available) {
        // append cluster (shouldn't reach here because DIR_FIND_FREE_SLOT appends a cluster when necessary),
        // but handle gracefully: append new cluster and set slot_cluster/slot_offset accordingly
        U32 new_cluster = FIND_NEXT_FREE_CLUSTER();
        if (new_cluster == 0) return FALSE;
        fat32[slot_cluster] = new_cluster;
        fat32[new_cluster] = FAT32_END_OF_CHAIN;
        if (!FAT_FLUSH()) return FALSE;
        MEMZERO(buf, CLUSTER_SIZE);
        slot_cluster = new_cluster;
        slot_offset = 0;
    }

    // write LFN entries then short entry
    U8 *dest = buf + slot_offset;
    if (num_of_lfn_entries) {
        MEMCPY(dest, LFNs, num_of_lfn_entries * sizeof(LFN));
        dest += num_of_lfn_entries * sizeof(LFN);
    }
    MEMCPY(dest, out, sizeof(DIR_ENTRY));

    // write cluster back
    if (!FAT_WRITE_CLUSTER(slot_cluster, buf)) return FALSE;

    return TRUE;
}

BOOLEAN CREATE_ROOT_DIR(U32 root_cluster) {
    U8 buf[CLUSTER_SIZE];
    MEMZERO(buf, CLUSTER_SIZE);

    DIR_ENTRY dot = {0};
    DIR_ENTRY dotdot = {0};

    MEMCPY(dot.FILENAME, ".          ", 11);
    dot.ATTRIB = FAT_ATTRB_DIR;
    dot.LOW_CLUSTER_BITS = root_cluster & 0xFFFF;
    dot.HIGH_CLUSTER_BITS = (root_cluster >> 16) & 0xFFFF;

    MEMCPY(dotdot.FILENAME, "..         ", 11);
    dotdot.ATTRIB = FAT_ATTRB_DIR;
    dotdot.LOW_CLUSTER_BITS = root_cluster & 0xFFFF;
    dotdot.HIGH_CLUSTER_BITS = (root_cluster >> 16) & 0xFFFF;

    MEMCPY(buf, &dot, sizeof(DIR_ENTRY));
    MEMCPY(buf + sizeof(DIR_ENTRY), &dotdot, sizeof(DIR_ENTRY));

    if (!FAT_WRITE_CLUSTER(root_cluster, buf)) return FALSE;

    // Mark root cluster as used
    fat32[root_cluster] = FAT32_END_OF_CHAIN;
    return FAT_FLUSH();
}



BOOLEAN CREATE_FSINFO() {
    FSINFO fsinfo;
    MEMZERO(&fsinfo, sizeof(fsinfo));

    fsinfo.SIGNATURE0  = 0x41615252;
    fsinfo.SIGNATURE1 = 0x61417272;
    fsinfo.FREE_CLUSTER_COUNT = 0xFFFFFFFF;
    fsinfo.CLUSTER_INDICATOR   = FIRST_ALLOWED_CLUSTER_NUMBER; // typically 2
    fsinfo.TRAIL_SIGNATURE  = 0xAA550000;

    return ATA_PIO_WRITE_CLUSTER(bpb.EXBR.FS_INFO_SECTOR, &fsinfo); 
}

BOOLEAN ZERO_INITIALIZE_FAT32(VOIDPTR BOOTLOADER_BIN, U32 sz) {
    if (!WRITE_DISK_BPB()) return FALSE;

    if (!POPULATE_BOOTLOADER(BOOTLOADER_BIN, sz)) return FALSE;
    if (!CREATE_FSINFO()) return FALSE;
    if (!INITIAL_WRITE_FAT()) return FALSE;
    if (!CREATE_ROOT_DIR(bpb.EXBR.ROOT_CLUSTER)) return FALSE;

    return TRUE;
}


BOOLEAN CREATE_CHILD_DIR(U32 parent_cluster, U8 *name, U8 attrib) {
    U32 new_cluster = FIND_NEXT_FREE_CLUSTER();
    if (!new_cluster) return FALSE;
    fat32[new_cluster] = FAT32_END_OF_CHAIN;
    if (!FAT_FLUSH()) return FALSE;

    // Create '.' and '..'
    U8 buf[CLUSTER_SIZE];
    MEMZERO(buf, CLUSTER_SIZE);

    DIR_ENTRY dot = {0};
    DIR_ENTRY dotdot = {0};

    MEMCPY(dot.FILENAME, ".          ", 11);
    dot.ATTRIB = FAT_ATTRB_DIR;
    dot.LOW_CLUSTER_BITS = new_cluster & 0xFFFF;
    dot.HIGH_CLUSTER_BITS = (new_cluster >> 16) & 0xFFFF;

    MEMCPY(dotdot.FILENAME, "..         ", 11);
    dotdot.ATTRIB = FAT_ATTRB_DIR;
    dotdot.LOW_CLUSTER_BITS = parent_cluster & 0xFFFF;
    dotdot.HIGH_CLUSTER_BITS = (parent_cluster >> 16) & 0xFFFF;

    MEMCPY(buf, &dot, sizeof(DIR_ENTRY));
    MEMCPY(buf + sizeof(DIR_ENTRY), &dotdot, sizeof(DIR_ENTRY));

    if (!ATA_PIO_WRITE_CLUSTER(new_cluster, buf)) return FALSE;

    attrib |= FAT_ATTRB_DIR;
    // Add directory entry for this new folder in parent
    DIR_ENTRY entry;
    if (!CREATE_DIR_ENTRY(parent_cluster, name, attrib, NULLPTR, 0, &entry))
        return FALSE;

    // Update entry cluster info
    entry.LOW_CLUSTER_BITS = new_cluster & 0xFFFF;
    entry.HIGH_CLUSTER_BITS = (new_cluster >> 16) & 0xFFFF;

    return TRUE;
}

BOOLEAN CREATE_CHILD_FILE(U32 parent_cluster, U8 *name, U8 attrib, PU8 filedata, U32 filedata_size) {
    // Add directory entry for this new folder in parent
    DIR_ENTRY entry;
    attrib |= FAT_ATTRIB_ARCHIVE;
    if (!CREATE_DIR_ENTRY(parent_cluster, name, attrib, filedata, filedata_size, &entry))
        return FALSE;
    return TRUE;
}

VOID FREE_FAT_FS_RESOURCES() {
    if(fat32) KFREE(fat32);
}

U32 GET_ROOT_CLUSTER() {
    return bpb.EXBR.ROOT_CLUSTER;
}

// U32 FIND_DIR_BY_NAME_AND_PARENT(U32 parent, U8 *name);
// U32 FIND_FILE_BY_NAME_AND_PARENT(U32 parent, U8 *name);
// VOID FIND_DIR_ENTRY_BY_NAME_AND_PARENT(DIR_ENTRY *out, U32 parent, U8 *name);
// VOIDPTR READ_FILE_CONTENTS(U32 size_out, DIR_ENTRY);
// BOOL DIR_ENUMERATE(U32 dir_cluster, DIR_ENTRY *out_entries, U32 max_count);
// BOOL DIR_REMOVE_ENTRY(U32 parent_cluster, const char *name);
// BOOL FAT_FREE_CHAIN(U32 start_cluster);
// BOOL FAT_TRUNCATE_CHAIN(U32 start_cluster, U32 new_size_clusters);
// BOOL FILE_READ(U32 start_cluster, U8 *buf, U32 size);
// BOOL FILE_WRITE(U32 start_cluster, const U8 *data, U32 size);
// BOOL FILE_APPEND(U32 start_cluster, const U8 *data, U32 size);
// U32 FILE_GET_SIZE(DIR_ENTRY *entry);
// U32 PATH_RESOLVE(const char *path);
// BOOLEAN PATH_CREATE_DIRS(const char *path);
// BOOL DIR_ENTRY_IS_FREE(DIR_ENTRY *entry);
// BOOL DIR_ENTRY_IS_DIR(DIR_ENTRY *entry);
// U32 DIR_ENTRY_CLUSTER(DIR_ENTRY *entry);
// VOID DIR_ENTRY_SET_CLUSTER(DIR_ENTRY *entry, U32 clust);