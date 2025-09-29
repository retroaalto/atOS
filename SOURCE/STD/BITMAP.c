#include <STD/BITMAP.h>
#include <STD/MEM.h>

BOOLEAN BITMAP_GET(BITMAP *bitmap, U32 index) {
    if (index >= bitmap->size * 8) return FALSE;
    U32 byteIndex = index / 8;
    U8 bitIndex = index % 8;
    return (bitmap->data[byteIndex] >> bitIndex) & 1u;
}

BOOLEAN BITMAP_SET(BITMAP *bitmap, U32 index, BOOLEAN value) {
    if (index >= bitmap->size * 8) return FALSE;
    U32 byteIndex = index / 8;
    U8 bitIndex = index % 8;
    U8 mask = 1u << bitIndex;

    if (value) {
        bitmap->data[byteIndex] |= mask;
    } else {
        bitmap->data[byteIndex] &= ~mask;
    }
    return TRUE;
}

VOID BITMAP_CREATE(U32 size, VOIDPTR buff_addr, BITMAP *bitmap) {
    if (!bitmap || !buff_addr) return;
    bitmap->data = (U8 *)buff_addr;
    bitmap->size = size;
    MEMZERO(bitmap->data, size);
}
