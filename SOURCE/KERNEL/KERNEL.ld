ENTRY(kernel_entry_main)

SECTIONS
{
    /* Start at 0x2000 */
    . = 0x2000;

    /* Text section (code) */
    .text :
    {
        *(.text)           /* All .text input sections */
        *(.text.*)         /* Any other .text subsections */
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (bss) */
    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
}
