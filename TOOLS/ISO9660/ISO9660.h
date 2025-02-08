#ifndef _ISO9660_H
#define _ISO9660_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef MSVC
#pragma pack(push, 1)
#endif

typedef struct _IsoPrimaryVolumeDescriptor {
    char volumeDescriptorType;
    char standardIdentifier[5];
    char version;
    char unused1;
    char systemIdentifier[32];
    char volumeIdentifier[32];
    char unused2[8];
    uint32_t volumeSpaceSize;
    char unused3[32];
    uint16_t volumeSetSize;
    uint16_t volumeSequenceNumber;
    uint16_t logicalBlockSize;
    uint32_t pathTableSize;
    uint32_t typeLPathTable;
    uint32_t optionalTypeLPathTable;
    uint32_t typeMPathTable;
    uint32_t optionalTypeMPathTable;
    char rootDirectoryRecord[34];
} 
#if defined(__linux__) || defined(__GNUC__) 
__attribute__((packed)) 
#endif
IsoPrimaryVolumeDescriptor;

#ifdef MSVC
#pragma pack(pop)
#endif

void read_primary_volume_descriptor(FILE *iso, IsoPrimaryVolumeDescriptor *pvd) {
    fseek(iso, 16 * 2048, SEEK_SET);
    fread(pvd, sizeof(IsoPrimaryVolumeDescriptor), 1, iso);
    if (strncmp(pvd->standardIdentifier, "CD001", 5) != 0) {
        printf("Not a valid ISO 9660 image.\n");
        return;
    }
}

#endif // _ISO9660_H