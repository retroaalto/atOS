/*+++
    Source/STD/STDIO.h - Standard Input/Output Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Standard input/output definitions for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/18 - Antonako1
        Initial version. Contains standard I/O function declarations.

REMARKS
    Use this header to include standard I/O functions in your application.
    This file should NOT be included in kernel or low-level drivers, 
        as it may introduce unwanted dependencies and conflicts, and
        may increase binary size.
    For kernel or low-level drivers, use low-level I/O driver functions instead.
---*/
#ifndef STDIO_H
#define STDIO_H

#include "./ATOSMINDEF.h" // TYPE DEFINITIONS
#include "../KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VBE.h" // VIDEO DRIVER, MEMORY AND CONSTANTS

#endif // STDIO_H
