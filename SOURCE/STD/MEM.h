/*+++
    STD\MEM.h - ATOS memory functions and constants

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.
---*/
#ifndef ATOSMEM_H
#define ATOSMEM_H

#include <STD/TYPES.h>

/*+++ 
    Memory size definitions
---*/

#define B               1
#define KB              1024
#define MB              1048576
#define GB              1073741824
#define TB              1099511627776

#define KB_TO_MB(x)    (x / KB)
#define MB_TO_KB(x)    (x * KB)

#define KB_TO_GB(x)    (x / MB)
#define GB_TO_KB(x)    (x * MB)

#define KB_TO_TB(x)    (x / GB)
#define TB_TO_KB(x)    (x * GB)

#define MB_TO_GB(x)    (x / KB)
#define GB_TO_MB(x)    (x * KB)

#define MB_TO_TB(x)    (x / GB)
#define TB_TO_MB(x)    (x * GB)

#define GB_TO_TB(x)    (x / GB)
#define TB_TO_GB(x)    (x * GB)

/*+++
    Memory functions
---*/



#endif // ATOSMEM_H