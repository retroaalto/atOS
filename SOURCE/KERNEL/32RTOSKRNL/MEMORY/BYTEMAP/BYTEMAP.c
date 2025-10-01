#include <MEMORY/BYTEMAP/BYTEMAP.h>
#include <STD/MEM.h>

PageState BYTEMAP_GET(BYTEMAP *bm, U32 index) {
    if (index >= bm->size) return PS_RESERVED; // or some safe default
    return (PageState)bm->data[index];
}

BOOLEAN BYTEMAP_SET(BYTEMAP *bm, U32 index, PageState state) {
    if (index >= bm->size) return FALSE;
    bm->data[index] = (U8)state;
    return TRUE;
}


VOID BYTEMAP_CREATE(U32 numPages, VOIDPTR buff_addr, BYTEMAP *bm) {
    if (!bm || !buff_addr) return;
    bm->data = (U8*)buff_addr;
    bm->size = numPages;
    MEMZERO(bm->data, numPages);
}
