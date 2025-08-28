#ifndef GDT_H
#define GDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#define KCODE_SEL 0x08
#define KDATA_SEL 0x10
typedef struct __attribute__((packed)) {
    U16 limit0;
    U16 base0;
    U8  base1;
    U8  access;
    U8  granularity;
    U8  base2;
} GDTENTRY;

typedef struct __attribute__((packed)) {
    U16 limit;
    U32 base;
} GDTDESCRIPTOR;
U0 GDT_INIT(U0);
#endif
