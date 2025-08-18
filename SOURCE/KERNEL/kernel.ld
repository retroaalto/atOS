/* kernel.ld - Linker script for atOS-RT 32-bit protected mode kernel entry */

OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    /* Kernel load address in memory */
    . = 0x2000;

    /* Code section */
    .text : {
        *(.text*)          /* all .text sections from object files */
    }

    /* Read-only data (string literals, constants) */
    .rodata : {
        *(.rodata*)        /* all .rodata sections */
    }

    /* Initialized data */
    .data : {
        *(.data*)          /* all .data sections */
    }

    /* Uninitialized data */
    .bss : {
        *(.bss*)           /* all .bss sections */
        *(COMMON)          /* common symbols (uninitialized globals) */
    }
}
