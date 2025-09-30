#ifndef BITMAP_H
#define BITMAP_H

#include <STD/TYPEDEF.h>

// Maximum bitmap size to handle up to 4GB of RAM with 4KB pages
#define TOTAL_PAGES (0x100000000 / 0x1000) // 4GB / 4KB
#define MAX_BITMAP_SIZE (TOTAL_PAGES / 8)

#define PAGE_FREE 0
#define PAGE_ALLOCATED 1
#define PAGE_RESERVED 2
#define PAGE_LOCKED 3

typedef struct {
    U8 *data;
    U32 size; // in bytes
} BITMAP;

VOID BITMAP_CREATE(U32 size, VOIDPTR buff_addr, BITMAP *bitmap);

BOOLEAN BITMAP_GET(BITMAP *bitmap, U32 index);
BOOLEAN BITMAP_SET(BITMAP *bitmap, U32 index, BOOLEAN value);

#endif // BITMAP_H