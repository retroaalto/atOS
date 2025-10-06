/*
 * STD/DEBUG.h
 *
 * Debugging utilities for programs and processes. 
 *
 * NOT usable in kernel code.
 */
#ifndef STD_DEBUG_H
#define STD_DEBUG_H

#include <STD/TYPEDEF.h>

void MEMORY_DUMP(const void* addr, U32 length);

#endif //STD_DEBUG_H