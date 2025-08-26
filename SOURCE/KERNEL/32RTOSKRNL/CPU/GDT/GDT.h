#ifndef GDT_H
#define GDT_H

#include "../../../../STD/ATOSMINDEF.h"
#include "../../MEMORY/MEMORY.h"

#define KCODE_SEL 0x08
#define KDATA_SEL 0x10

U0 GDT_INIT(U0);
#endif
