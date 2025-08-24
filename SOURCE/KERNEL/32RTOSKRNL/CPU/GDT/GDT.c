/*+++
    Source/KERNEL/32RTOSKRNL/CPU/GDT/GDT.c - GDT Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit GDT and IDT implementations for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/23 - Antonako1
        Initial version. Contains GDT structures and constants.

REMARKS

    Only for kernel usage.

---*/
#include "./GDT.h"

BOOLEAN GDT_INIT(U0) {
    asm volatile("cli");

    // Initialize the GDT
    return TRUE;
}
