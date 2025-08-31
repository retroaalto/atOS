#ifndef BITMAP_H
#define BITMAP_H

#include <STD/TYPEDEF.h>

typedef struct {
    U8 *data;
    U32 size; // in bytes
} BITMAP;

VOID BITMAP_CREATE(U32 size, VOIDPTR buff_addr, BITMAP *bitmap);

BOOLEAN BITMAP_GET(BITMAP *bitmap, U32 index);
BOOLEAN BITMAP_SET(BITMAP *bitmap, U32 index, BOOLEAN value);

#endif // BITMAP_H