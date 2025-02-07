/*+++
    STD\IO\I.c - Input functions for Standard I/O Library

    Part of atOS-RT
    
    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    This module contains common Input functions

AUTHORS
    Antonako1

REVISION HISTORY
    2025/02/07 - Antonako1
        Initial version

REMARKS
    Code is shared into to parts: BIOS side and drivers.

    BIOS side contains functions that can be used before drivers are loaded.
    BIOS code is made in assembly, and its it has a wrapper in C.

    Drivers side contains functions that can be used after drivers are loaded.
    
    For BIOS part, search for BIOS_START and BIOS_END.
    For drivers part, search for DRIVERS_START and DRIVERS_END.
---*/
#include <STD/IO/I.h>

/*+++ BIOS_START ---*/
/*+++ BIOS_END ---*/

/*+++ DRIVERS_START ---*/
/*+++ DRIVERS_END ---*/