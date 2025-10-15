#include <DRIVERS/ATA_PIO/ATA_PIO.h>
#include <DRIVERS/CMOS/CMOS.h>
#include <FS/FAT/FAT.h>
#include <STD/MEM.h>
#include <HEAP/KHEAP.h>
#include <STD/STRING.h>

static BPB bpb ATTRIB_DATA = { 0 };
static BOOL bpb_loaded ATTRIB_DATA = FALSE;

#define SET_BPB(x, val, sz) MEMCPY(&x, val, sz)

BOOLEAN LOAD_BPB() { 
    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    if(!ATA_PIO_READ_SECTORS(0, 1, buf)) {
        KFREE(buf);
        return FALSE;
    }
    MEMCPY(&bpb, buf, sizeof(BPB));
    KFREE(buf);
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
    SET_BPB(bpb.EXBR.VOLUME_LABEL, "ATOS RT    ", 11);
    SET_BPB(bpb.EXBR.FILE_SYSTEM_TYPE, "FAT32   ", 8);

    U32 seccount = ATA_CALC_SEC_COUNT(sizeof(BPB));
    U8 *buf = KMALLOC(ATA_PIO_SECTOR_SIZE);
    if(!buf) return FALSE;
    MEMZERO(buf, ATA_PIO_SECTOR_SIZE);
    MEMCPY(buf, GET_BPB(), sizeof(BPB));
    U32 res = ATA_PIO_WRITE_SECTORS(0, seccount, buf);
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

// U32 GET_DATA_REGION_SECTOR

// U32 CALCULATE_LBA(U32 cluster) {
    // return (cluster )
// }