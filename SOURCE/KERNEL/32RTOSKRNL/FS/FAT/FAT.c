#include <DRIVERS/ATA_PIO/ATA_PIO.h>
#include <DRIVERS/CMOS/CMOS.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <FS/FAT/FAT.h>
#include <FS/ISO9660/ISO9660.h>
#include <HEAP/KHEAP.h>
#include <PROC/PROC.h>
#include <STD/MEM.h>
#include <STD/MATH.h>
#include <STD/STRING.h>
#include <STD/BINARY.h>

static BPB bpb ATTRIB_DATA = { 0 };
static U32 *fat32 ATTRIB_DATA = NULLPTR;
static DIR_ENTRY *root_dir = NULLPTR;
static U32 root_dir_end ATTRIB_DATA = 0;
static BOOL bpb_loaded ATTRIB_DATA = 0;
static FSINFO fsinfo ATTRIB_DATA = { 0 };

#define SET_BPB(x, val, sz) MEMCPY(&x, val, sz)

#define GET_CLUSTER_SIZE() (bpb.BYTES_PER_SECTOR * bpb.SECTORS_PER_CLUSTER)
#define GET_CLUSTERS_NEEDED(cluster_size, bytes)( (bytes + cluster_size - 1) / cluster_size)
#define GET_CLUSTERS_NEEDED_IN_BYTES(clusters_needed, cluster_size) (clusters_needed * (cluster_size))


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

    // Make sure CLUSTER_SIZE matches sectors per cluster
    if (bpb.SECTORS_PER_CLUSTER * 512 != CLUSTER_SIZE) return FALSE;

    U32 data_start_sector = bpb.RESERVED_SECTORS + (bpb.NUM_OF_FAT * bpb.EXBR.SECTORS_PER_FAT);
    U32 sector = data_start_sector + (cluster - FIRST_ALLOWED_CLUSTER_NUMBER) * bpb.SECTORS_PER_CLUSTER;

    // Optionally, check sector range against total disk sectors
    // if (sector + bpb.SECTORS_PER_CLUSTER > TOTAL_DISK_SECTORS) return FALSE;

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
BOOLEAN GET_BPB_LOADED(){ 
    // TODO: read from disk and check ids, if IDs, LOAD_BPB()
    return TRUE; 
}


BPB *GET_BPB() { 
    if(!bpb_loaded) {
        if(!LOAD_BPB()) return NULLPTR;
    } 
    return &bpb;
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

static U32 last_allocated_cluster = 2;

U32 FIND_NEXT_FREE_CLUSTER() {
    U32 fat_sectors = bpb.EXBR.SECTORS_PER_FAT;
    U32 bytes_per_fat = fat_sectors * bpb.BYTES_PER_SECTOR;
    U32 total_clusters = bytes_per_fat / 4;

    for(U32 offset = 0; offset < total_clusters - 2; offset++) {
        U32 i = 2 + (last_allocated_cluster - 2 + offset) % (total_clusters - 2);
        if(fat32[i] == FAT32_FREE_CLUSTER) {
            last_allocated_cluster = i + 1;
            return i;
        }
    }
    return 0; // no free cluster
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

static BOOLEAN DIR_NAME_COMP(const DIR_ENTRY *entry, const U8 *name) {
    if (!entry || !name) return FALSE;

    DIR_ENTRY tmp;
    MEMZERO(&tmp, sizeof(tmp));
    FAT_83FILENAMEFY(&tmp, (U8*)name); // fills tmp.FILENAME[11] with upper-case 8.3

    // Compare 11 bytes (8 name + 3 ext)
    return (MEMCMP(entry->FILENAME, tmp.FILENAME, 11) == 0);
}

static BOOLEAN LFN_MATCH(U8 *name, LFN *lfns, U32 count) {
    if (!name || !lfns || count == 0) return FALSE;

    U32 pos = 0;
    for (U32 i = 0; i < count; i++) {
        LFN *e = &lfns[i];
        // NAME_1 (5 chars)
        for (U32 j = 0; j < 5 && e->NAME_1[j] != 0 && pos < FAT_MAX_FILENAME; j++, pos++)
            if ((U8)e->NAME_1[j] != name[pos]) return FALSE;
        // NAME_2 (6 chars)
        for (U32 j = 0; j < 6 && e->NAME_2[j] != 0 && pos < FAT_MAX_FILENAME; j++, pos++)
            if ((U8)e->NAME_2[j] != name[pos]) return FALSE;
        // NAME_3 (2 chars)
        for (U32 j = 0; j < 2 && e->NAME_3[j] != 0 && pos < FAT_MAX_FILENAME; j++, pos++)
            if ((U8)e->NAME_3[j] != name[pos]) return FALSE;
    }

    return name[pos] == '\0';
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

    // Empty file: no clusters needed
    if (sz == 0) {
        out_ent->LOW_CLUSTER_BITS  = 0;
        out_ent->HIGH_CLUSTER_BITS = 0;
        return TRUE;
    }

    // Calculate how many clusters are needed
    U32 clusters_needed = (sz + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    U32 allocated_clusters[clusters_needed]; // store allocated clusters for rollback
    U32 allocated_count = 0;
    U32 first_cluster = 0;
    U32 prev_cluster  = 0;

    PU8 src = filedata;
    U32 remaining = sz;

    for (U32 i = 0; i < clusters_needed; i++) {
        U32 c = FIND_NEXT_FREE_CLUSTER();
        if (c == 0) {
            // Out of clusters, rollback allocated ones
            for (U32 j = 0; j < allocated_count; j++) {
                fat32[allocated_clusters[j]] = FAT32_FREE_CLUSTER;
            }
            FAT_FLUSH();
            return FALSE;
        }

        // Save allocated cluster
        allocated_clusters[allocated_count++] = c;

        // Link previous cluster
        if (prev_cluster != 0) {
            fat32[prev_cluster] = c;
        } else {
            first_cluster = c; // first cluster of the file
        }
        prev_cluster = c;

        // Write cluster data
        U32 tocopy = (remaining > CLUSTER_SIZE) ? CLUSTER_SIZE : remaining;

        // Allocate temporary buffer only for last cluster if partially filled
        PU8 buf = src;
        U8 last_cluster_buf[CLUSTER_SIZE]; // only used if partial last cluster
        if (tocopy < CLUSTER_SIZE) {
            MEMZERO(last_cluster_buf, CLUSTER_SIZE);
            MEMCPY(last_cluster_buf, src, tocopy);
            buf = last_cluster_buf;
        }

        if (!FAT_WRITE_CLUSTER(c, buf)) {
            // rollback on failure
            for (U32 j = 0; j < allocated_count; j++) fat32[allocated_clusters[j]] = FAT32_FREE_CLUSTER;
            FAT_FLUSH();
            return FALSE;
        }

        src       += tocopy;
        remaining -= tocopy;
    }

    // Mark last cluster as end-of-chain
    fat32[prev_cluster] = FAT32_END_OF_CHAIN;

    // Flush FAT to disk
    if (!FAT_FLUSH()) return FALSE;

    // Set directory entry start cluster
    out_ent->LOW_CLUSTER_BITS  = first_cluster & 0xFFFF;
    out_ent->HIGH_CLUSTER_BITS = (first_cluster >> 16) & 0xFFFF;

    return TRUE;
}


BOOLEAN CREATE_DIR_ENTRY(U32 parent_cluster, U8 *FILENAME, U8 ATTRIB, PU8 filedata, U32 filedata_size, DIR_ENTRY *out) {
    if (!out || !FILENAME) return FALSE;

    // Check if the name already exists in the parent directory
    DIR_ENTRY temp;
    if (FIND_DIR_ENTRY_BY_NAME_AND_PARENT(&temp, parent_cluster, FILENAME)) {
        // Duplicate name exists
        return FALSE;
    }
    MEMZERO(out, sizeof(DIR_ENTRY));
    out->ATTRIB = ATTRIB;
    FAT_83FILENAMEFY(out, FILENAME); // convert to 8.3
    FAT_CREATION_UPDATETIMEDATE(out);
    out->CREATION_TIME_HUN_SEC = (get_ticks() % 199) + 1;

    // Prepare LFN entries if needed
    U32 num_of_lfn_entries = 0;
    LFN LFNs[MAX_LFN_COUNT];
    MEMZERO(LFNs, sizeof(LFNs));
    if (STRLEN(FILENAME) > 11) {
        num_of_lfn_entries = (STRLEN(FILENAME) + CHARS_PER_LFN - 1) / CHARS_PER_LFN;
        FILL_LFNs(LFNs, num_of_lfn_entries, FILENAME);
    }
    
    // Allocate clusters and write file data if present
    if (filedata_size > 0 && filedata) {
        out->FILE_SIZE = filedata_size;
        if (!WRITE_FILEDATA(out, filedata, filedata_size)) return FALSE;
    } else {
        out->FILE_SIZE = 0;
        out->LOW_CLUSTER_BITS = 0;
        out->HIGH_CLUSTER_BITS = 0;
    }

    // Find free slot in parent directory
    U32 slot_cluster = 0;
    U32 slot_offset = 0;
    if (!DIR_FIND_FREE_SLOT(parent_cluster, &slot_cluster, &slot_offset)) return FALSE;

    // Read cluster
    U8 buf[CLUSTER_SIZE];
    if (!FAT_READ_CLUSTER(slot_cluster, buf)) return FALSE;

    // Ensure space for LFN + short entry
    U32 available = CLUSTER_SIZE - slot_offset;
    U32 required = (num_of_lfn_entries * sizeof(LFN)) + sizeof(DIR_ENTRY);
    if (required > available) {
        // Append new cluster if necessary
        U32 new_cluster = FIND_NEXT_FREE_CLUSTER();
        if (new_cluster == 0) return FALSE;
        fat32[slot_cluster] = new_cluster;
        fat32[new_cluster] = FAT32_END_OF_CHAIN;
        if (!FAT_FLUSH()) return FALSE;
        MEMZERO(buf, CLUSTER_SIZE);
        slot_cluster = new_cluster;
        slot_offset = 0;
    }

    // Write LFN entries then short entry
    U8 *dest = buf + slot_offset;
    if (num_of_lfn_entries) {
        MEMCPY(dest, LFNs, num_of_lfn_entries * sizeof(LFN));
        dest += num_of_lfn_entries * sizeof(LFN);
    }
    MEMCPY(dest, out, sizeof(DIR_ENTRY));

    // Write cluster back
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

// Recursive function to copy ISO directories and files
BOOLEAN copy_iso_to_fat32(IsoDirectoryRecord *iso_dir, U32 parent_cluster) {
    if (!iso_dir) return FALSE;

    U32 count = 0;
    U8 success;
    IsoDirectoryRecord **dir_contents = ISO9660_GET_DIR_CONTENTS(iso_dir, FALSE, &count, &success);
    if(!success) return FALSE;
    if (count == 0) return TRUE;
    for (U32 i = 0; i < count; i++) {
        IsoDirectoryRecord *rec = dir_contents[i];
        if (!rec || rec->length == 0) continue;

        CHAR name[ISO9660_MAX_PATH];
        MEMZERO(name, sizeof(name));
        U32 copy_len = rec->fileNameLength < ISO9660_MAX_PATH - 1 ? rec->fileNameLength : ISO9660_MAX_PATH - 1;
        MEMCPY(name, rec->fileIdentifier, copy_len);
        name[copy_len] = '\0';
        if (rec->fileFlags & ISO9660_FILE_FLAG_DIRECTORY) {
            // Create directory in FAT32
            U32 cluster_out = 0;
            if (!CREATE_CHILD_DIR(parent_cluster, (U8 *)name, FAT_ATTRB_DIR, &cluster_out)) {
                ISO9660_FREE_MEMORY(dir_contents);
                HLT;
                return FALSE;
            }
            
            if (!copy_iso_to_fat32(rec, cluster_out)) {
                ISO9660_FREE_MEMORY(dir_contents);
                return FALSE;
            }

        } else {
            // Read file contents from ISO
            U8 *file_data = ISO9660_READ_FILEDATA_TO_MEMORY(rec);
            if (!file_data) {
                ISO9660_FREE_MEMORY(dir_contents);
                return FALSE;
            }
            
            U32 cluster_out = 0;
            // Create file in FAT32
            name[rec->fileNameLength - 2] = '\0';
            if (!CREATE_CHILD_FILE(parent_cluster, (U8 *)name, FAT_ATTRIB_ARCHIVE, file_data, rec->extentLengthLE, &cluster_out)) {
                ISO9660_FREE_MEMORY(file_data);
                ISO9660_FREE_MEMORY(dir_contents);
                return FALSE;
            }
            ISO9660_FREE_MEMORY(file_data);
        }

    }

    ISO9660_FREE_LIST(dir_contents, count);
    return TRUE;
}

BOOLEAN COPY_ISO9660_CONTENTS_TO_FAT32() {
    PrimaryVolumeDescriptor pvd;
    if (!ISO9660_READ_PVD(&pvd, sizeof(PrimaryVolumeDescriptor))) return FALSE;
    IsoDirectoryRecord root_iso;
    ISO9660_EXTRACT_ROOT_FROM_PVD(&pvd, &root_iso);
    return copy_iso_to_fat32(&root_iso, bpb.EXBR.ROOT_CLUSTER);
}

BOOLEAN ZERO_INITIALIZE_FAT32(VOIDPTR BOOTLOADER_BIN, U32 sz) {
    if (!WRITE_DISK_BPB()) return FALSE;

    if (!POPULATE_BOOTLOADER(BOOTLOADER_BIN, sz)) return FALSE;
    if (!CREATE_FSINFO()) return FALSE;
    if (!INITIAL_WRITE_FAT()) return FALSE;
    if (!CREATE_ROOT_DIR(bpb.EXBR.ROOT_CLUSTER)) return FALSE;
    if(!COPY_ISO9660_CONTENTS_TO_FAT32()) return FALSE;
    return TRUE;
}

BOOLEAN CREATE_CHILD_DIR(U32 parent_cluster, U8 *name, U8 attrib, U32 *cluster_out) {
    *cluster_out = 0;
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
    *cluster_out = new_cluster; 
    return TRUE;
}

BOOLEAN CREATE_CHILD_FILE(U32 parent_cluster, U8 *name, U8 attrib, PU8 filedata, U32 filedata_size, U32 *cluster_out) {
    // Add directory entry for this new folder in parent
    DIR_ENTRY entry;
    attrib |= FAT_ATTRIB_ARCHIVE;
    if (!CREATE_DIR_ENTRY(parent_cluster, name, attrib, filedata, filedata_size, &entry))
        return FALSE;
    *cluster_out = (entry.HIGH_CLUSTER_BITS << 16) | entry.LOW_CLUSTER_BITS;
    return TRUE;
}

VOID FREE_FAT_FS_RESOURCES() {
    if(fat32) KFREE(fat32);
}

U32 GET_ROOT_CLUSTER() {
    return bpb.EXBR.ROOT_CLUSTER;
}

BOOLEAN READ_LFNS(DIR_ENTRY *ent, LFN *out, U32 *size_out) {
    if (!ent || !out || !size_out) return FALSE;

    *size_out = 0;

    // Short entries are usually immediately after LFN entries in reverse order
    // We'll scan backwards in memory from the DIR_ENTRY location (assume contiguous cluster buffer)
    LFN *ptr = (LFN *)ent;
    I32 count = 0;

    while (count < MAX_LFN_COUNT) {
        ptr--; // move to previous 32-byte entry
        if (((DIR_ENTRY *)ptr)->ATTRIB != FAT_ATTRIB_LFN)
            break; // no more LFN entries

        out[count] = *(LFN *)ptr; // copy LFN entry
        count++;

        if (out[count - 1].ORDER & LFN_LAST_ENTRY)
            break; // reached first LFN in chain
    }

    if (count == 0)
        return FALSE;

    // Reverse order to get the correct sequence
    for (I32 i = 0; i < count / 2; i++) {
        LFN temp = out[i];
        out[i] = out[count - 1 - i];
        out[count - 1 - i] = temp;
    }

    *size_out = (U32)count;
    return TRUE;
}

BOOLEAN FIND_DIR_ENTRY_BY_CLUSTER_NUMBER(U32 cluster, DIR_ENTRY *out) {
    if (!out || cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U8 buf[CLUSTER_SIZE];
    U32 current_cluster = cluster;

    while (current_cluster >= FIRST_ALLOWED_CLUSTER_NUMBER && current_cluster < FAT32_END_OF_CHAIN) {
        if (!FAT_READ_CLUSTER(current_cluster, buf)) return FALSE;

        for (U32 offset = 0; offset < CLUSTER_SIZE; offset += sizeof(DIR_ENTRY)) {
            DIR_ENTRY *entry = (DIR_ENTRY *)(buf + offset);

            if (entry->FILENAME[0] == 0x00) {
                // No more entries in this directory
                return FALSE;
            }

            if (entry->FILENAME[0] == 0xE5 || entry->ATTRIB == FAT_ATTRIB_LFN) {
                // Skip deleted entries and LFN entries
                continue;
            }

            U32 entry_cluster = ((U32)entry->HIGH_CLUSTER_BITS << 16) | entry->LOW_CLUSTER_BITS;
            if (entry_cluster == cluster) {
                MEMCPY(out, entry, sizeof(DIR_ENTRY));
                return TRUE;
            }
        }

        // Move to next cluster in directory chain
        current_cluster = fat32[current_cluster];
        if (current_cluster >= FAT32_END_OF_CHAIN) break;
    }

    return FALSE;
}

BOOLEAN FIND_DIR_ENTRY_BY_NAME_AND_PARENT(DIR_ENTRY *out, U32 parent_cluster, U8 *name) {
    if (!out || !name) return FALSE;

    DIR_ENTRY entries[MAX_CHILD_ENTIES];
    if (!DIR_ENUMERATE(parent_cluster, entries, MAX_CHILD_ENTIES)) return FALSE;

    for (U32 i = 0; i < MAX_CHILD_ENTIES; i++) {
        if (entries[i].FILENAME[0] == 0x00) break; // no more entries
        if (entries[i].ATTRIB == FAT_ATTRIB_LFN) continue; // skip LFN entries

        LFN lfns[MAX_LFN_COUNT];
        U32 lfn_count = 0;
        if (READ_LFNS(&entries[i], lfns, &lfn_count) && lfn_count > 0) {
            if (LFN_MATCH(name, lfns, lfn_count)) {
                MEMCPY(out, &entries[i], sizeof(DIR_ENTRY));
                return TRUE;
            }
        } else {
            // Compare short 8.3 name
            U8 shortname[13];
            MEMZERO(shortname, 13);
            // convert 8.3 to string
            for (U32 j = 0; j < 8 && entries[i].FILENAME[j] != ' '; j++) shortname[j] = entries[i].FILENAME[j];
            if (entries[i].FILENAME[8] != ' ') {
                shortname[STRLEN(shortname)] = '.';
                for (U32 j = 0; j < 3 && entries[i].FILENAME[8+j] != ' '; j++)
                    shortname[STRLEN(shortname)] = entries[i].FILENAME[8+j];
            }
            if (STRCMP(shortname, name) == TRUE) {
                MEMCPY(out, &entries[i], sizeof(DIR_ENTRY));
                return TRUE;
            }
        }
    }

    return FALSE;
}

U32 FIND_DIR_BY_NAME_AND_PARENT(U32 parent_cluster, U8 *name) {
    DIR_ENTRY e;
    if (FIND_DIR_ENTRY_BY_NAME_AND_PARENT(&e, parent_cluster, name)) {
        if (IS_FLAG_SET(e.ATTRIB, FAT_ATTRB_DIR)) {
            return ((U32)e.HIGH_CLUSTER_BITS << 16) | e.LOW_CLUSTER_BITS;
        }
    }
    return 0;
}

U32 FIND_FILE_BY_NAME_AND_PARENT(U32 parent_cluster, U8 *name) {
    DIR_ENTRY e;
    if (FIND_DIR_ENTRY_BY_NAME_AND_PARENT(&e, parent_cluster, name)) {
        if (!IS_FLAG_SET(e.ATTRIB, FAT_ATTRB_DIR)) {
            return ((U32)e.HIGH_CLUSTER_BITS << 16) | e.LOW_CLUSTER_BITS;
        }
    }
    return 0;
}

VOIDPTR READ_FILE_CONTENTS(U32 *size_out, DIR_ENTRY *ent) {
    VOIDPTR buf = NULL;
    *size_out = 0;
    if(IS_FLAG_UNSET(ent->ATTRIB, FAT_ATTRIB_ARCHIVE)) return buf;

    U32 file_size = ent->FILE_SIZE;
    if (file_size == 0)
        return buf;


    U32 clust = ((U32)ent->HIGH_CLUSTER_BITS << 16) | ent->LOW_CLUSTER_BITS;
    if (clust < FIRST_ALLOWED_CLUSTER_NUMBER)
        return buf;
    
    U32 cluster_size = GET_CLUSTER_SIZE();
    U32 clusters_needed = GET_CLUSTERS_NEEDED(cluster_size, file_size);
    U32 bytes = GET_CLUSTERS_NEEDED_IN_BYTES(clusters_needed, cluster_size);
    buf = KMALLOC(bytes);
    if(!buf) return NULL;
    MEMZERO(buf, bytes);

    U32 total_read = 0;
    U32 current_cluster = 0;
    U8 *tmp_clust = KMALLOC(cluster_size);
    if(!tmp_clust) {
        KFREE(buf);
        return NULL;
    }
    MEMZERO(tmp_clust, bytes);

    while (current_cluster >= FIRST_ALLOWED_CLUSTER_NUMBER &&
           current_cluster < FAT32_END_OF_CHAIN &&
           total_read < file_size) {

        // Read current cluster
        if (!ATA_PIO_READ_CLUSTER(current_cluster, tmp_clust)) {
            KFREE(buf);
            KFREE(tmp_clust);
            return NULL;
        }

        // Copy data into output buffer
        U32 bytes_to_copy = MIN(cluster_size, file_size - total_read);
        MEMCPY(buf + total_read, tmp_clust, bytes_to_copy);
        total_read += bytes_to_copy;

        // Move to next cluster in chain
        current_cluster = fat32[current_cluster];
    }

    *size_out = total_read;
    return buf;
}

U32 FAT_CLUSTER_TO_LBA(U32 cluster) {
    if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return 0; // invalid cluster
    U32 first_data_sector = bpb.RESERVED_SECTORS + (bpb.NUM_OF_FAT * bpb.EXBR.SECTORS_PER_FAT);
    return first_data_sector + (cluster - FIRST_ALLOWED_CLUSTER_NUMBER) * bpb.SECTORS_PER_CLUSTER;
}
U32 FAT_GET_NEXT_CLUSTER(U32 cluster) {
    if (cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return 0;  // reserved/invalid
    U32 val = fat32[cluster];
    if (val >= FAT32_END_OF_CHAIN) return 0;               // end of chain
    if (val == FAT32_BAD_CLUSTER) return 0;                // bad cluster
    return val;
}
BOOL DIR_ENUMERATE_LFN(U32 dir_cluster, FAT_LFN_ENTRY *out_entries, U32 max_count) {
    if (!out_entries || max_count == 0) return FALSE;

    U32 filled = 0;
    U8 sector_buf[BYTES_PER_SECT];
    LFN lfn_entries[MAX_LFN_COUNT];
    U32 lfn_count = 0;

    while (dir_cluster >= 2 && !FAT32_IS_EOC(dir_cluster)) {
        U32 lba = FAT_CLUSTER_TO_LBA(dir_cluster);

        // iterate over each sector
        for (U32 s = 0; s < SECT_PER_CLUST; s++) {
            if (!FAT_READ_CLUSTER(lba + s, sector_buf)) return FALSE;

            // each sector has 16 entries (512 / 32)
            for (U32 i = 0; i < ENTRIES_PER_SECTOR; i++) {
                U8 *ptr = sector_buf + (i * sizeof(DIR_ENTRY));
                DIR_ENTRY *ent = (DIR_ENTRY *)ptr;

                if (ent->FILENAME[0] == 0x00) {
                    // End of directory
                    return TRUE;
                }

                if (ent->FILENAME[0] == 0xE5) {
                    // Deleted entry, skip and reset LFN chain
                    lfn_count = 0;
                    continue;
                }

                if (ent->ATTRIB == FAT_ATTRIB_LFN) {
                    // This is an LFN part — store it
                    LFN *lfn = (LFN *)ptr;
                    if (lfn_count < MAX_LFN_COUNT) {
                        lfn_entries[lfn_count++] = *lfn;
                    }
                    continue;
                }

                // Normal entry (file or directory)
                if (filled >= max_count)
                    return TRUE;

                // Build full LFN (if any collected)
                CHAR name[FAT_MAX_FILENAME];
                MEMZERO(name, FAT_MAX_FILENAME);

                if (lfn_count > 0) {
                    // Sort/rebuild from last to first LFN
                    for (I32 idx = (I32)lfn_count - 1; idx >= 0; idx--) {
                        LFN *lfn = &lfn_entries[idx];
                        U16 *segments[3] = { lfn->NAME_1, lfn->NAME_2, lfn->NAME_3 };
                        U32 seg_lengths[3] = { 5, 6, 2 };

                        for (U32 si = 0; si < 3; si++) {
                            for (U32 ci = 0; ci < seg_lengths[si]; ci++) {
                                U16 ch = segments[si][ci];
                                if (ch == 0x0000 || ch == 0xFFFF)
                                    goto done_lfn;
                                U32 len = STRLEN(name);
                                if (len < FAT_MAX_FILENAME - 1)
                                    name[len] = (CHAR)(ch & 0xFF); // ASCII truncation
                                else
                                    goto done_lfn;
                            }
                        }
                    }
                done_lfn:;
                } else {
                    // No LFN entries — use short 8.3
                    CHAR short_name[13];
                    U32 k = 0;
                    for (U32 j = 0; j < 8 && ent->FILENAME[j] != ' '; j++)
                        short_name[k++] = ent->FILENAME[j];
                    if (ent->FILENAME[8] != ' ')
                        short_name[k++] = '.';
                    for (U32 j = 8; j < 11 && ent->FILENAME[j] != ' '; j++)
                        short_name[k++] = ent->FILENAME[j];
                    short_name[k] = '\0';
                    STRCPY(name, short_name);
                }

                // Fill entry
                MEMCPY(&out_entries[filled].entry, ent, sizeof(DIR_ENTRY));
                STRCPY(out_entries[filled].lfn, name);
                filled++;

                // Reset LFN collector
                lfn_count = 0;
            }
        }

        // Move to next cluster
        dir_cluster = FAT_GET_NEXT_CLUSTER(dir_cluster);
    }

    return TRUE;
}

/// @brief Enumerates all valid directory entries in a FAT32 directory cluster chain.
/// @param dir_cluster Starting cluster of the directory to read.
/// @param out_entries Preallocated array of DIR_ENTRY to fill.
/// @param max_count Maximum number of entries to write into out_entries.
/// @return TRUE if the enumeration was successful, FALSE on failure.
BOOL DIR_ENUMERATE(U32 dir_cluster, DIR_ENTRY *out_entries, U32 max_count) {
    if (!out_entries || max_count == 0) return FALSE;

    U32 entries_filled = 0;
    U32 current_cluster = dir_cluster;
    U8 cluster_buf[CLUSTER_SIZE]; // buffer to read a cluster

    while (FAT32_IS_VALID(current_cluster) && entries_filled < max_count) {
        MEMZERO(cluster_buf, sizeof(cluster_buf));

        // Read the cluster into memory
        if (!FAT_READ_CLUSTER(current_cluster, cluster_buf)) return FALSE;

        // Each cluster has multiple DIR_ENTRYs
        for (U32 i = 0; i < CLUSTER_SIZE / sizeof(DIR_ENTRY); i++) {
            DIR_ENTRY *ent = (DIR_ENTRY *)(cluster_buf + i * sizeof(DIR_ENTRY));

            // Skip free or deleted entries
            if (DIR_ENTRY_IS_FREE(ent)) continue;

            // Copy valid entry into output array
            MEMCPY(&out_entries[entries_filled], ent, sizeof(DIR_ENTRY));
            entries_filled++;

            if (entries_filled >= max_count) break;
        }

        // Advance to the next cluster in FAT
        current_cluster = FAT_GET_NEXT_CLUSTER(current_cluster);
        if (FAT32_IS_EOC(current_cluster)) break; // end of chain
    }

    return TRUE;
}

U32 FILE_GET_SIZE(DIR_ENTRY *entry) {
    return entry->FILE_SIZE;
}

CHAR *STRTOK_R(CHAR *str, const CHAR *delim, CHAR **saveptr) {
    CHAR *start;

    if (str)
        start = str;
    else if (*saveptr)
        start = *saveptr;
    else
        return NULLPTR;

    // Skip leading delimiters
    while (*start && STRCHR(delim, *start)) start++;

    if (*start == '\0') {
        *saveptr = NULLPTR;
        return NULLPTR;
    }

    CHAR *token_end = start;
    while (*token_end && !STRCHR(delim, *token_end)) token_end++;

    if (*token_end) {
        *token_end = '\0';
        *saveptr = token_end + 1;
    } else {
        *saveptr = NULLPTR;
    }

    return start;
}

BOOL DIR_ENTRY_IS_FREE(DIR_ENTRY *entry) {
    if (!entry) return TRUE;
    // Free if first byte is 0x00 (never used) or 0xE5 (deleted)
    return (entry->FILENAME[0] == 0x00 || entry->FILENAME[0] == 0xE5);
}

BOOLEAN PATH_RESOLVE_ENTRY(U8 *path, FAT_LFN_ENTRY *out_entry) {
    if (!path || !out_entry) return FALSE;

    MEMZERO(out_entry, sizeof(FAT_LFN_ENTRY));

    U32 current_cluster = GET_ROOT_CLUSTER();
    out_entry->lfn[0] = '\0';

    U8 path_copy[FAT_MAX_PATH];
    STRNCPY(path_copy, path, FAT_MAX_PATH - 1);
    path_copy[FAT_MAX_PATH - 1] = '\0';

    U8 *token = path_copy;
    if (token[0] == '/') token++;  // skip leading '/'

    U8 *component = STRTOK_R(token, "/", &token);
    while (component) {
        DIR_ENTRY entries[MAX_CHILD_ENTIES];
        U32 count = MAX_CHILD_ENTIES;
        if (!DIR_ENUMERATE(current_cluster, entries, count)) return FALSE;

        BOOLEAN found = FALSE;
        for (U32 i = 0; i < count; i++) {
            DIR_ENTRY *e = &entries[i];

            if (DIR_ENTRY_IS_FREE(e)) continue;

            U8 name[FAT_MAX_FILENAME];
            MEMZERO(name, sizeof(name));
            if (!READ_LFNS(e, (LFN *)name, (U32[]){sizeof(name)})) {
                STRNCPY(name, (U8 *)e->FILENAME, 11);
                name[11] = '\0';
            }

            if (STRCASECMP(name, component) == 0) {
                MEMCPY(&out_entry->entry, e, sizeof(DIR_ENTRY));
                STRNCPY(out_entry->lfn, name, FAT_MAX_FILENAME - 1);
                current_cluster = (e->HIGH_CLUSTER_BITS << 16) | e->LOW_CLUSTER_BITS;
                found = TRUE;
                break;
            }
        }

        if (!found) return FALSE;
        component = STRTOK_R(NULL, "/", &token);
    }

    return TRUE;
}

BOOL DIR_ENTRY_IS_DIR(DIR_ENTRY *entry) {
    if(IS_FLAG_SET(entry->ATTRIB, FAT_ATTRB_DIR)) return TRUE;
    return FALSE;
}
U32 DIR_ENTRY_CLUSTER(DIR_ENTRY *entry) {
    U32 res = (entry->HIGH_CLUSTER_BITS << 16) | entry->LOW_CLUSTER_BITS;
    return res;
}

// Frees the entire FAT chain starting from start_cluster
BOOL FAT_FREE_CHAIN(U32 start_cluster) {
    if (start_cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U32 cluster = start_cluster;
    while (cluster >= FIRST_ALLOWED_CLUSTER_NUMBER && cluster < FAT32_END_OF_CHAIN) {
        U32 next = FAT_GET_NEXT_CLUSTER(cluster);
        fat32[cluster] = FAT32_FREE_CLUSTER; // mark as free
        cluster = next;
    }

    return FAT_FLUSH();
}

// Truncates a FAT chain to new_size_clusters, freeing the rest
BOOL FAT_TRUNCATE_CHAIN(U32 start_cluster, U32 new_size_clusters) {
    if (start_cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;
    if (new_size_clusters == 0) return FAT_FREE_CHAIN(start_cluster);

    U32 cluster = start_cluster;
    U32 count = 1;

    while (cluster >= FIRST_ALLOWED_CLUSTER_NUMBER && cluster < FAT32_END_OF_CHAIN) {
        U32 next = FAT_GET_NEXT_CLUSTER(cluster);
        if (count == new_size_clusters) {
            // truncate here
            fat32[cluster] = FAT32_END_OF_CHAIN;
            // free remaining chain
            cluster = next;
            while (cluster >= FIRST_ALLOWED_CLUSTER_NUMBER && cluster < FAT32_END_OF_CHAIN) {
                U32 tmp = FAT_GET_NEXT_CLUSTER(cluster);
                fat32[cluster] = FAT32_FREE_CLUSTER;
                cluster = tmp;
            }
            break;
        }
        cluster = next;
        count++;
    }

    return FAT_FLUSH();
}


// Overwrite a file completely with new data
BOOL FILE_WRITE(DIR_ENTRY *entry, const U8 *data, U32 size) {
    if (!entry || !data) return FALSE;

    // Free existing cluster chain if any
    U32 start_cluster = (entry->HIGH_CLUSTER_BITS << 16) | entry->LOW_CLUSTER_BITS;
    if (start_cluster >= FIRST_ALLOWED_CLUSTER_NUMBER) {
        FAT_FREE_CHAIN(start_cluster);  // marks clusters as free in FAT
    }

    // Write new data into clusters
    if (!WRITE_FILEDATA(entry, data, size)) return FALSE;

    // Update file size
    entry->FILE_SIZE = size;
    FAT_UPDATETIMEDATE(entry);

    return TRUE;
}

// Append data to existing file
BOOL FILE_APPEND(DIR_ENTRY *entry, const U8 *data, U32 size) {
    if (!entry || !data || size == 0) return FALSE;

    // Determine starting cluster
    U32 start_cluster = (entry->HIGH_CLUSTER_BITS << 16) | entry->LOW_CLUSTER_BITS;
    if (start_cluster == 0) {
        // File was empty, just use FILE_WRITE
        return FILE_WRITE(entry, data, size);
    }

    // Determine last cluster in chain and used bytes
    U32 last_cluster = FAT32_GetLastCluster(start_cluster);
    U32 used_bytes = entry->FILE_SIZE % CLUSTER_SIZE;
    U32 remaining = size;
    const U8 *src = data;
    U8 buf[CLUSTER_SIZE];

    // Fill the partially used last cluster
    if (used_bytes > 0) {
        if (!FAT_READ_CLUSTER(last_cluster, buf)) return FALSE;
        U32 to_copy = MIN(CLUSTER_SIZE - used_bytes, remaining);
        MEMCPY(buf + used_bytes, src, to_copy);
        if (!FAT_WRITE_CLUSTER(last_cluster, buf)) return FALSE;
        src += to_copy;
        remaining -= to_copy;
    }

    // Allocate new clusters if needed
    while (remaining > 0) {
        U32 new_cluster = FIND_NEXT_FREE_CLUSTER();
        if (new_cluster == 0) return FALSE; // out of space
        fat32[last_cluster] = new_cluster;
        fat32[new_cluster] = FAT32_END_OF_CHAIN;
        if (!FAT_FLUSH()) return FALSE;

        MEMZERO(buf, CLUSTER_SIZE);
        U32 to_copy = MIN(CLUSTER_SIZE, remaining);
        MEMCPY(buf, src, to_copy);
        if (!FAT_WRITE_CLUSTER(new_cluster, buf)) return FALSE;

        src += to_copy;
        remaining -= to_copy;
        last_cluster = new_cluster;
    }

    // Update file size
    entry->FILE_SIZE += size;
    FAT_UPDATETIMEDATE(entry);

    return TRUE;
}

// Removes a directory entry named `name` in the directory pointed by `entry`.
// Marks the entry as deleted and frees its clusters.
BOOL DIR_REMOVE_ENTRY(DIR_ENTRY *dir_entry, const char *name) {
    if (!dir_entry || !name) return FALSE;

    U32 dir_cluster = (dir_entry->HIGH_CLUSTER_BITS << 16) | dir_entry->LOW_CLUSTER_BITS;
    if (dir_cluster < FIRST_ALLOWED_CLUSTER_NUMBER) return FALSE;

    U8 cluster_buf[CLUSTER_SIZE];

    while (dir_cluster >= FIRST_ALLOWED_CLUSTER_NUMBER && dir_cluster < FAT32_END_OF_CHAIN) {
        if (!FAT_READ_CLUSTER(dir_cluster, cluster_buf)) return FALSE;

        for (U32 offset = 0; offset < CLUSTER_SIZE; offset += sizeof(DIR_ENTRY)) {
            DIR_ENTRY *ent = (DIR_ENTRY *)(cluster_buf + offset);

            // End of directory
            if (ent->FILENAME[0] == 0x00) return FALSE;

            // Skip deleted entries or long filename entries
            if (ent->FILENAME[0] == 0xE5 || ent->ATTRIB == FAT_ATTRIB_LFN)
                continue;

            // Compare short name
            if (STRCASECMP((const char *)ent->FILENAME, name) == 0) {
                // Free clusters if it’s a file or directory
                U32 start_cluster = (ent->HIGH_CLUSTER_BITS << 16) | ent->LOW_CLUSTER_BITS;
                if (start_cluster >= FIRST_ALLOWED_CLUSTER_NUMBER) {
                    FAT_FREE_CHAIN(start_cluster);
                }

                // Mark directory entry as deleted
                ent->FILENAME[0] = 0xE5;

                // Write cluster back
                if (!FAT_WRITE_CLUSTER(dir_cluster, cluster_buf)) return FALSE;
                return TRUE;
            }
        }

        // Move to next cluster in the directory chain
        dir_cluster = fat32[dir_cluster];
    }

    return FALSE; // not found
}
