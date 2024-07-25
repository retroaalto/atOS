ENTRY(kernel_main)
SECTIONS
{
    . = 1M;  /* Start the kernel code at 1MB */
    .text : {
        *(.text)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }
}
