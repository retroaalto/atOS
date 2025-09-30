/*+++
    <PATH> - <ABOUT>

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Describe

AUTHORS
    Name(s)

REVISION HISTORY
    yyyy/mm/dd - Name(s)
        Description

REMARKS
    Additional remarks, if any.
---*/
#ifndef UHEAP_H
#define UHEAP_H
#include <STD/TYPEDEF.h>

typedef struct USER_HEAP_BLOCK {
    U32 size;
    BOOL free;
    struct USER_HEAP_BLOCK *next;
} USER_HEAP_BLOCK;

VOIDPTR UMALLOC(U32 size);
VOID UFREE(VOIDPTR ptr);
BOOLEAN UREALLOC(VOIDPTR *addr, U32 oldSize, U32 newSize);
VOIDPTR UCALLOC(U32 num, U32 size);

#endif // UHEAP_H