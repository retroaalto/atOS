#include <STD/MEM.h>

U0 *MEMCPY(U0* dest, CONST U0* src, U32 size) {
    for(U32 i = 0; i < size; i++) {
        ((U8*)dest)[i] = ((U8*)src)[i];
    }
    return dest;
}
U0 *MEMSET(U0* dest, U8 value, U32 size) {
    for(U32 i = 0; i < size; i++) {
        ((U8*)dest)[i] = value;
    }
    return dest;
}
U0 *MEMZERO(U0* dest, U32 size) {
    MEMSET(dest, 0, size);
    return dest;
}
U0 *MEMCMP(CONST U0* ptr1, CONST U0* ptr2, U32 size) {
    for(U32 i = 0; i < size; i++) {
        if(((U8*)ptr1)[i] != ((U8*)ptr2)[i]) {
            return (U0*)1;
        }
    }
    return (U0*)0;
}
U0 *MEMMOVE(U0* dest, CONST U0* src, U32 size) {
    if (dest < src || dest >= src + size) {
        // Non-overlapping regions, safe to use memcpy
        MEMCPY(dest, src, size);
    } else {
        // Overlapping regions, copy backwards
        for (U32 i = size; i > 0; i--) {
            ((U8*)dest)[i - 1] = ((U8*)src)[i - 1];
        }
    }
    return dest;
}
