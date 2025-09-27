#ifndef ISO9660_H
#define ISO9660_H

#include "../../../STD/TYPEDEF.h"
#include "../../DRIVERS/DISK/ATA_ATAPI.h"

#define ISO9660_SECTOR_SIZE 2048
#define ISO9660_VOLUME_DESCRIPTOR_START 16
#define ISO9660_PVD_OFFSET ((ISO9660_VOLUME_DESCRIPTOR_START*ISO9660_SECTOR_SIZE)/ISO9660_SECTOR_SIZE)
#define ISO9660_MAX_PATH 256

typedef U8 strD;
typedef U8 strA;

typedef struct _DateTime {
    strD year[4];
    strD month[2];
    strD day[2];
    strD hour[2];
    strD minute[2];
    strD second[2];
    strD ms[2];
    U8 gmtOffset;
}
__attribute__((packed))
DateTime;

typedef struct _BootRecord {
    U8 type;
    U8 id[5];
    U8 version;
    U8 bootSystemIdentifier[32];
    U8 bootIdentifier[32];
    U8 bootSystemUse[1977];
}
__attribute__((packed))
BootRecord;

typedef struct _IsoDirectoryDateAndTime {
    U8 year;
    U8 month;
    U8 day;
    U8 hour;
    U8 minute;
    U8 second;
    I8 gmtOffset;
}
__attribute__((packed))
IsoDirectoryDateAndTime;

typedef U8 fileFlag;
// Directory Record
typedef struct _IsoDirectoryRecord {
    U8 length;                                         // +0
    U8 extendedAttributeRecordLength;                  // +1
    U32 extentLocationLE_LBA;                          // +2
    U32 extentLocationBE_LBA;                          // +6
    U32 extentLengthLE;                                // +10
    U32 extentLengthBE;                                // +14
    IsoDirectoryDateAndTime recordingDateAndTime;           // +18
    fileFlag fileFlags;                                     // +25
    U8 fileUnitSize;                                   // +26
    U8 interleaveGapSize;                              // +27
    U16 volumeSequenceNumberLE;                        // +28
    U16 volumeSequenceNumberBE;                        // +30
    U8 fileNameLength;                                 // +31
    strD fileIdentifier[1];
} 
__attribute__((packed))
IsoDirectoryRecord;

#define IDR_SIZE sizeof(_IsoDirectoryRecord)

typedef struct _PrimaryVolumeDescriptor {
    U8 TypeCode;
    strA standardIdentifier[5];
    U8 version;
    U8 unused1;
    strA systemIdentifier[32];
    strD volumeIdentifier[32];
    U8 unused2[8];
    U32 volumeSpaceSizeLE;
    U32 volumeSpaceSizeBE;
    U8 unused3[32];
    U16 volumeSetSizeLE;
    U16 volumeSetSizeBE;
    U16 volumeSequenceNumberLE;
    U16 volumeSequenceNumberBE;
    U16 logicalBlockSizeLE;
    U16 logicalBlockSizeBE;
    U32 pathTableSizeLE;
    U32 pathTableSizeBE;
    U32 pathTableLocationLE;
    U32 optionalPathTableLocationLE;
    U32 LocationMPathBE;
    U32 optionalPathTableLocationBE;
    IsoDirectoryRecord rootDirectoryRecord;
    strD volumeSetIdentifier[128];
    strA publisherIdentifier[128];
    strA dataPreparerIdentifier[128];
    strA applicationIdentifier[128];
    strD copyrightFileIdentifier[37];
    strD abstractFileIdentifier[37];
    strD bibliographicFileIdentifier[37];
    DateTime creationDateAndTime;
    DateTime modificationDateAndTime;
    DateTime expirationDateAndTime;
    DateTime effectiveDateAndTime;
    U8 fileStructureVersion;
    U8 unused4;
    U8 applicationUsed[512];
    U8 reserved[653];
}
__attribute__((packed))
PrimaryVolumeDescriptor;

#ifndef ISO9660_ONLY_DEFINES
BOOLEAN ISO9660_IMAGE_CHECK(U0);
BOOLEAN ISO9660_READ_PRIMARYVOLUMEDESCRIPTOR(PrimaryVolumeDescriptor *descriptor);
BOOLEAN ISO9660_NORMALIZE_PATH(CHAR *path);
BOOLEAN ISO9660_READ_DIRECTORY_RECORD(CHAR *path, IsoDirectoryRecord *record, U32 size);
BOOLEAN ISO9660_READFILE_TO_MEMORY(CHAR *path, U0 *outBuffer, U32 outSize);

#endif

#endif // ISO9660_H