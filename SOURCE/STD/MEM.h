/*+++
    Source/STD/MEM.h - Memory Management Definitions

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Memory management definitions for atOS.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/19 - Antonako1
        Initial version. Contains memory management definitions.
    2025/08/30 - Antonako1
        Added memory management functions.
REMARKS
---*/
#ifndef MEM_H
#define MEM_H

#include <STD/TYPEDEF.h>

/// @brief Copy memory area.
/// @param dest Pointer to destination memory area.
/// @param src Pointer to source memory area.
/// @param size Number of bytes to copy.
/// @return Pointer to destination memory area.
U0 *MEMCPY(U0* dest, CONST U0* src, U32 size);


U0 *MEMSET(U0* dest, U8 value, U32 size);
U0 *MEMZERO(U0* dest, U32 size);

// Optimized version
U0 *MEMCPY_OPT(U0* dest, CONST U0* src, U32 size); 

// Returns 0 if equal, non-zero if not equal
BOOLEAN MEMCMP(CONST U0* ptr1, CONST U0* ptr2, U32 size);

U0 *MEMMOVE(U0* dest, CONST U0* src, U32 size);



// DEVNOTE: Every pointer allocated can be shared across processes!
U0 *MAlloc(U32 size);
U0 *CAlloc(U32 num, U32 size);
U0 *ReAlloc(U0* ptr, U32 oldSize, U32 newSize);
VOID Free(U0* ptr);

#endif // MEM_H