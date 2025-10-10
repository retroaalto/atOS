#ifndef FAT32_H
#define FAT32_H

typedef struct {
    U32 cluster;      ///< Cluster number
} FAT32_CLUSTER;

#ifndef FAT32_ONLY_DEFINES
#endif // FAT32_ONLY_DEFINES

#endif // FAT32_H