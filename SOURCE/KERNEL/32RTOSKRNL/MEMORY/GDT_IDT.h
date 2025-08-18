/*+++
    SOURCE/KERNEL/32RTOSKRNL/MEMORY/GDT_IDT.h - GDT and IDT Definitions

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.
---*/
// typedef struct __attribute__((packed)) _GDTR32 {
//     U16 limit;
//     U32 base;
// } GDTR32;

// static void show_gdt_info(void) {
//     GDTR32 gdtr;
//     __asm__ volatile ("sgdt %0" : "=m"(gdtr));
//     print_string("[GDT]\n");
//     print_label_hex("  GDTR.limit", (U32)gdtr.limit);
//     print_label_hex("  GDTR.base ", gdtr.base);
// }