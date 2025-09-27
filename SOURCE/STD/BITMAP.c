#include <STD/BITMAP.h>
#include <STD/BINARY.h>
#include <STD/MEM.h>

BOOLEAN BITMAP_GET(BITMAP *bitmap, U32 index) {
    if(index > bitmap->size * 8) return FALSE;
    U32 byteIndex = index / 8;
    U8 bitIndex = index % 8;
    U8 bitIndexer = SHR(0b10000000, bitIndex);
    if ((bitmap->data[byteIndex] & bitIndexer) > 0) {
        return TRUE;
    }
    return FALSE;
}

BOOLEAN BITMAP_SET(BITMAP *bitmap, U32 index, BOOLEAN value) {
    if(index > bitmap->size * 8) return FALSE;
    U32 byteIndex = index / 8;
    U8 bitIndex = index % 8;
    U8 bitIndexer = SHR(0b10000000, bitIndex);
    bitmap->data[byteIndex] &= ~bitIndexer;
    if(value) {
        bitmap->data[byteIndex] |= bitIndexer;
    }
    return TRUE;
}

VOID BITMAP_CREATE(U32 size, VOIDPTR buff_addr, BITMAP *bitmap) {
    if (!bitmap || !buff_addr) return;
    bitmap->data = (U8 *)buff_addr;
    bitmap->size = size;
    MEMZERO(bitmap->data, size);
}
