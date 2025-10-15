#ifndef FAT_H
#define FAT_H

#include <STD/TYPEDEF.h>

/// @brief FAT32 Extended BIOS Parameter Block (EXBR) structure
typedef struct ATTRIB_PACKED {
    U32 SECTORS_PER_FAT;       ///< Number of sectors per FAT
    U16 EXT_FLAGS;             ///< FAT32 extended flags
    U16 FS_VERSION;            ///< FAT32 file system version
    U32 ROOT_CLUSTER;          ///< First cluster of root directory
    U16 FS_INFO_SECTOR;        ///< FSInfo sector number
    U16 BACKUP_BOOT_SECTOR;    ///< Backup boot sector location
    U8 RESERVED[12];           ///< Reserved
    U8 DRIVE_NUMBER;           ///< BIOS drive number
    U8 RESERVED1;              ///< Reserved
    U8 BOOT_SIGNATURE;         ///< Boot signature (0x29)
    U32 VOLUME_ID;             ///< Volume serial number
    U8 VOLUME_LABEL[11];       ///< Volume label
    U8 FILE_SYSTEM_TYPE[8];    ///< File system type string (e.g., "FAT32   ")
} EXBR_FAT32;

/// @brief BIOS Parameter Block (BPB) structure, common for FAT12/16/32
typedef struct ATTRIB_PACKED  {
    U8 JMPSHORT[3];            ///< Jump instruction to boot code
    U8 OEM_IDENT[8];            ///< OEM identifier string
    U16 BYTES_PER_SECTOR;       ///< Bytes per sector (usually 512)
    U8 SECTORS_PER_CLUSTER;     ///< Sectors per cluster
    U16 RESERVED_SECTORS;       ///< Reserved sectors before FAT
    U8 NUM_OF_FAT;              ///< Number of FAT copies
    U16 NUM_OF_ROOT_DIR_ENTRIES;///< Number of root directory entries (FAT12/16)
    U16 TOTAL_SECTORS;          ///< Total sectors (small count, use LARGE_SECTOR_COUNT if 0)
    U8 MEDIA_DESCR_TYPE;        ///< Media descriptor byte
    U16 NUM_OF_SECTORS_PER_FAT; ///< Sectors per FAT (FAT12/16)
    U16 NUM_OF_SECTORS_PER_TRACK;///< Sectors per track (for CHS addressing)
    U16 NUM_OF_HEADS;            ///< Number of heads (for CHS addressing)
    U32 NUM_OF_HIDDEN_SECTORS;   ///< Hidden sectors before partition
    U32 LARGE_SECTOR_COUNT;      ///< Total sectors (FAT32 or >65535 for FAT12/16)
    EXBR_FAT32    EXBR;    ///< FAT32-specific fields
} BPB;

/// @brief FAT32 FSInfo structure (sector providing free cluster count and next free cluster)
typedef struct ATTRIB_PACKED {
    U32 SIGNATURE0;             ///< First signature (0x41615252)
    U8 RES0[480];               ///< Reserved
    U32 SIGNATURE1;             ///< Second signature (0x61417272)
    U32 FREE_CLUSTER_COUNT;     ///< Number of free clusters on volume
    U32 CLUSTER_INDICATOR;      ///< Next free cluster to allocate
    U8 RES1[12];                ///< Reserved
    U32 TRAIL_SIGNATURE;        ///< Last signature (0xAA550000)
} FSINFO;

/// @brief Standard 32-byte FAT directory entry
typedef struct ATTRIB_PACKED {
    U8 FILENAME[11];            ///< Short 8.3 filename (padded with spaces)
    U8 ATTRIB;                  ///< File attributes (archive, dir, read-only, etc.)
    U8 RES;                     ///< Reserved
    U8 CREATION_TIME_HUN_SEC;   ///< Hundredths of a second at creation (0-199)
    U16 CREATION_TIME;           ///< Encoded creation time (hour 5bit, min 6bit, sec/2 5bit)
    U16 CREATION_DATE;           ///< Encoded creation date (year since 1980 7bit, month 4bit, day 5bit)
    U16 LAST_ACCESS_DATE;        ///< Last access date
    U16 FIRST_CLUSTER_BITS;      ///< High 16 bits of first cluster (FAT32 only)
    U16 LAST_MOD_TIME;           ///< Last modification time
    U16 LAST_MOD_DATE;           ///< Last modification date
    U16 LAST_CLUSTER_BITS;       ///< Low 16 bits of first cluster
    U32 FILE_SIZE;               ///< File size in bytes
} DIR_ENTRY;

/// @brief Max filename and extension lengths
#define MAX_FILENAME_LEN    8
#define MAX_FILENAME_EXT    3

/// @brief FAT file attribute flags
#define FAT_ATTRIB_READ_ONLY    0x01
#define FAT_ATTRIB_HIDDEN       0x02
#define FAT_ATTRIB_SYSTEM       0x04
#define FAT_ATTRIB_VOL_ID       0x08
#define FAT_ATTRB_DIR           0x10
#define FAT_ATTRIB_LFN          (FAT_ATTRIB_READ_ONLY|FAT_ATTRIB_HIDDEN|FAT_ATTRIB_SYSTEM|FAT_ATTRIB_VOL_ID)

/// @brief Encode hour, minute, second into FAT 16-bit time
#define SET_TIME(h, m, s) \
    (U16)((((h) & 0x1F) << 11) | (((m) & 0x3F) << 5) | (((s) / 2) & 0x1F))

/// @brief Encode year, month, day into FAT 16-bit date
#define SET_DATE(y, m, d) \
    (U16)(((((y) - 1980) & 0x7F) << 9) | (((m) & 0x0F) << 5) | ((d) & 0x1F))

/// @brief Long File Name (LFN) directory entry (32 bytes)
typedef struct ATTRIB_PACKED  {
    U8  ORDER;           ///< Sequence number (bit 6 = 1 if last entry)
    U16 NAME_1[5];       ///< First 5 UTF-16 characters of the long name
    U8  ATTR;            ///< Must be 0x0F
    U8  TYPE;            ///< Must be 0x00
    U8  CHECKSUM;        ///< Checksum of short 8.3 name
    U16 NAME_2[6];       ///< Next 6 UTF-16 characters
    U16 FIRST_CLUSTER;   ///< Always 0x0000
    U16 NAME_3[2];       ///< Final 2 UTF-16 characters
} LFN;

/// @brief FAT path and name limits
#define FAT_MAX_PATH     255
#define FAT_MAX_FILENAME 255
#define FAT_MAX_EXT      10
#define MAX_NESTED_DIRS  16
/// @brief FAT type enumeration
typedef enum {
    FAT16,
} FAT_TYPE;



#ifndef FAT_ONLY_DEFINES

/// @brief Returns the current BPB type (FAT12/16/32)
U32 GET_BPB_TYPE();

/// @brief Returns pointer to current BPB structure
BPB *GET_BPB();

/// @brief Loads BPB from disk
BOOLEAN LOAD_BPB();

/// @brief Write BPB to disk
/// @return TRUE if successful
BOOLEAN WRITE_DISK_BPB();

/// @brief Populate bootloader sector with boot code and BPB
/// @param BOOTLOADER_BIN Pointer to bootloader binary
/// @param sz Size of bootloader
/// @return TRUE if successful
BOOLEAN POPULATE_BOOTLOADER(VOIDPTR BOOTLOADER_BIN, U32 sz);


#endif // FAT_ONLY_DEFINES
#endif // FAT_H
