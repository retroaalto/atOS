/*+++
ISO 9660 reader

This program reads the primary volume descriptor of an ISO 9660 image and prints
the volume identifier and volume space size.

Usage: ISO9660 <iso_image> [<file_name>]

Compile with:
    gcc -o ISO9660 ISO9660.c -Wall -Wextra
    cl ISO9660.c /I. /W4

Run with:
    ISO9660 <iso_image> [<file_name>]

---*/

// #define DEBUG 1

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


const char strD_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
const char strA_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_!\"%&'()*+,-./:;<=>?";

typedef char strD;
typedef char strA;

#define DATE_TIME_PATTERN "%.*s-%.*s-%.*s %.*s:%.*s:%.*s.%.*s GMT%+03d:00"
#define DATE_TIME_FIELDS(dt) 4, dt.year, 2, dt.month, 2, dt.day, 2, dt.hour, 2, dt.minute, 2, dt.second, 2, dt.ms, dt.gmtOffset

typedef struct _DateTime {
    strD year[4];
    strD month[2];
    strD day[2];
    strD hour[2];
    strD minute[2];
    strD second[2];
    strD ms[2];
    uint8_t gmtOffset;
}
#if defined(__linux__) || defined(__GNUC__)
__attribute__((packed))
#endif
DateTime;

typedef struct _BootRecord {
    uint8_t type;
    uint8_t id[5];
    uint8_t version;
    uint8_t bootSystemIdentifier[32];
    uint8_t bootIdentifier[32];
    uint8_t bootSystemUse[1977];
}
#if defined(__linux__) || defined(__GNUC__)
__attribute__((packed))
#endif
BootRecord;

typedef struct _IsoDirectoryDateAndTime {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int8_t gmtOffset;
}
#if defined(__linux__) || defined(__GNUC__)
__attribute__((packed))
#endif
IsoDirectoryDateAndTime;

#define DIR_ENTRY_DATE_TIME_PATTERN "%02d/%02d/%02d %02d:%02d:%02d GMT%+03d:00"
#define DIR_ENTRY_DATE_TIME_FIELDS(dt) 1900 + dt.year, dt.month, dt.day, dt.hour+1, 1+dt.minute, dt.second+1, dt.gmtOffset
typedef uint8_t fileFlag;
// Directory Record
typedef struct _IsoDirectoryRecord {
    uint8_t length;
    uint8_t extendedAttributeRecordLength;
    uint32_t extentLocationLE_LBA;
    uint32_t extentLocationBE_LBA;
    uint32_t extentLengthLE;
    uint32_t extentLengthBE;
    IsoDirectoryDateAndTime recordingDateAndTime;
    fileFlag fileFlags;
    uint8_t fileUnitSize;
    uint8_t interleaveGapSize;
    uint16_t volumeSequenceNumberLE;
    uint16_t volumeSequenceNumberBE;
    uint8_t fileNameLength;
    strD fileIdentifier[1];
} 
#if defined(__linux__) || defined(__GNUC__)
__attribute__((packed))
#endif
IsoDirectoryRecord;

typedef struct _PrimaryVolumeDescriptor {
    uint8_t TypeCode;
    strA standardIdentifier[5];
    uint8_t version;
    uint8_t unused1;
    strA systemIdentifier[32];
    strD volumeIdentifier[32];
    uint8_t unused2[8];
    uint32_t volumeSpaceSizeLE;
    uint32_t volumeSpaceSizeBE;
    uint8_t unused3[32];
    uint16_t volumeSetSizeLE;
    uint16_t volumeSetSizeBE;
    uint16_t volumeSequenceNumberLE;
    uint16_t volumeSequenceNumberBE;
    uint16_t logicalBlockSizeLE;
    uint16_t logicalBlockSizeBE;
    uint32_t pathTableSizeLE;
    uint32_t pathTableSizeBE;
    uint32_t pathTableLocationLE;
    uint32_t optionalPathTableLocationLE;
    uint32_t LocationMPathBE;
    uint32_t optionalPathTableLocationBE;
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
    uint8_t fileStructureVersion;
    uint8_t unused4;
    uint8_t applicationUsed[512];
    uint8_t reserved[653];
}
#if defined(__linux__) || defined(__GNUC__)
__attribute__((packed))
#endif
PrimaryVolumeDescriptor;

#define SECTOR_SIZE 2048
#define MAX_PATH 256

// Reads the Primary Volume Descriptor
void read_primary_volume_descriptor(FILE *iso, PrimaryVolumeDescriptor *pvd) {
    fseek(iso, 16 * SECTOR_SIZE, SEEK_SET);
    fread(pvd, sizeof(PrimaryVolumeDescriptor), 1, iso);
    
    if (strncmp((char*)pvd->standardIdentifier, "CD001", 5) != 0) {
        printf("Not a valid ISO 9660 image.\n");
        exit(1);
    }
}

void normalize_path(char *path) {
    for (int i = 0; path[i]; i++) {
        path[i] = toupper(path[i]);

        if (path[i] == ';') {
            path[i] = '\0';
            break;
        }
    }
}

bool read_directory(FILE *iso, uint32_t lba, uint32_t size, char *target, char *original_target) {
    fseek(iso, lba * size, SEEK_SET);
    char buffer[SECTOR_SIZE];
    fread(buffer, SECTOR_SIZE, 1, iso);
    printf("Target: '%s'\n", target);
    uint32_t offset = 0;
    while (offset < size) {
        IsoDirectoryRecord *record = (IsoDirectoryRecord *)(buffer + offset);
        if (record->length == 0) break;

        char name[256];
        strncpy(name, record->fileIdentifier, record->fileNameLength);
        name[record->fileNameLength] = '\0';
        normalize_path(name);
        #if DEBUG
        printf("File name: '%s', "BYTE_TO_BINARY_PATTERN"\n", name, BYTE_TO_BINARY(record->fileFlags));
        #endif
        if (record->fileFlags & 0b00000010) {
            if (strcmp(name, target) == 0) {
                printf("Directory found: %s\n", name);

                // path/to/file.txt -> to/file.txt
                char *slash = strchr(original_target, '/');
                if(!slash) {
                    printf("File not found\n");
                    return false;
                }
                memmove(original_target, slash + 1, strlen(slash));
                #if DEBUG
                printf("Searching for file '%s' in directory '%s'\n", original_target, name);
                #endif
                memcpy(target, original_target, strlen(original_target));
                slash = strchr(target, '/');
                if (slash) {
                    *slash = '\0';
                } else {
                    target[strlen(original_target)] = '\0';
                }

                return read_directory(iso, record->extentLocationLE_LBA, record->extentLengthLE, target, original_target);
            }
        } else {
            if (strcmp(name, target) == 0) {
                printf("File found: %s\n", name);
                char buffer[1024];
                fseek(iso, record->extentLocationLE_LBA * size, SEEK_SET);
                fread(buffer, 1024, 1, iso);
                printf("'%.*s'\n", 1024, buffer);
                return true;
            }
        }

        offset += record->length;
    }
    return false;
}

// void path_table_search(FILE *iso, PrimaryVolumeDescriptor *pvd, char *target){
//     printf("Path Table Size: %d\n", pvd->pathTableSizeLE);
//     printf("Path Table Location: %d\n", pvd->pathTableLocationLE);
//     printf("Logical Block Size: %d\n", pvd->logicalBlockSizeLE);

//     fseek(iso, pvd->pathTableLocationLE * pvd->logicalBlockSizeLE, SEEK_SET);
//     char buffer[SECTOR_SIZE];
//     fread(buffer, SECTOR_SIZE, 1, iso);
//     uint32_t offset = 0;
    
// }

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iso_image> [<file_name>]\n", argv[0]);
        return 1;
    }

    FILE *iso = fopen(argv[1], "rb");
    if (!iso) {
        fprintf(stderr, "Failed to open ISO file: %s\n", argv[1]);
        return 1;
    }

    printf("Reading ISO 9660 image: %s\n", argv[1]);
    printf("%llu\n", sizeof(PrimaryVolumeDescriptor));
    PrimaryVolumeDescriptor pvd;
    read_primary_volume_descriptor(iso, &pvd);
    if (argc < 3) {
        printf("No file name specified, exiting...\n");
        fclose(iso);
        return 0;
    }

    // Search for the file in the root directory
    char *filename = (char*)malloc(MAX_PATH);
    strncpy(filename, argv[2], MAX_PATH);
    normalize_path(filename);

    char *original_filename = (char*)malloc(MAX_PATH);
    strncpy(original_filename, argv[2], MAX_PATH);
    normalize_path(original_filename);

    // path/to/file.txt -> path
    char *slash = strchr(filename, '/');
    if (slash) {
        *slash = '\0';
    }
    read_directory(iso, pvd.rootDirectoryRecord.extentLocationLE_LBA, pvd.rootDirectoryRecord.extentLengthLE, filename, original_filename);

    // path_table_search(iso, &pvd, filename);
    
    free(filename);

    fclose(iso);
    return 0;
}
