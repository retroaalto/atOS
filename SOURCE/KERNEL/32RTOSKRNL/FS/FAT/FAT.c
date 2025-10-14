#include <DRIVERS/ATA_PIO/ATA_PIO.h>
#include <DRIVERS/CMOS/CMOS.h>
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
    SET_BPB(bpb.EXBR.FAT32.VOLUME_LABEL, "ATOS RT    ", 11);
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
    SET_BPB(bpb.EXBR.FAT16.VOLUME_LABEL, "ATOS RT    ", 11);
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
    SET_BPB(bpb.EXBR.FAT12.VOLUME_LABEL, "ATOS RT    ", 11);
    SET_BPB(bpb.EXBR.FAT12.FILE_SYSTEM_TYPE, "FAT12   ", 8);
}

BOOLEAN WRITE_DISK_BPB(FAT_TYPE TYPE) {
    TYPE = FAT32;// :)
    
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