#ifndef FAT32_H
#define FAT32_H

typedef struct {
    U32 cluster;      ///< Cluster number
} FAT32_CLUSTER;

#define FAT32_MAX_PATH 255

#ifndef FAT32_ONLY_DEFINES
#endif // FAT32_ONLY_DEFINES

#endif // FAT32_H