#ifndef GDT_H
#define GDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#define GDT_START MEM_GDT_BASE
#define GDT_END   MEM_GDT_END
#define GDT_SIZE  (GDT_END - GDT_START + 1)

// Access
#define ACC_KCODE 0x9A
#define ACC_KDATA 0x92
#define ACC_UCODE 0xFA
#define ACC_UDATA 0xF2
#define ACC_TSS   0x89  // if you ever add TSS

// Flags for 32-bit segments (G=1 D=1 L=0 AVL=0)
#define GRAN_32_4K 0xCF

typedef struct __attribute__((packed)) {
    U16 Limit0;
    U16 Base0;
    U8  Base1;
    U8  AccessByte;
    U8  Limit1_Flags;
    U8  Base2;
} GDTENTRY;

typedef struct __attribute__((packed)) {
    U16 size;
    U32 offset;
} GDTDESCRIPTOR;

// Indices / selectors
#define GDT_IDX_NULL   0
#define GDT_IDX_KCODE  1
#define GDT_IDX_KDATA  2

#define KCODE_SEL ((GDT_IDX_KCODE << 3) | 0)  // 0x08
#define KDATA_SEL ((GDT_IDX_KDATA << 3) | 0)  // 0x10

// Number of GDT entries we actually use (no TSS)
#define GDT_ENTRY_COUNT 3

VOID GDT_INIT(VOID);

#endif // GDT_H
