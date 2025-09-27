// /* TSS.c - Inline-ASM-only TSS + Task Gate setup for double-fault (vector 8)
//  *
//  * Place this file in your kernel build and call tss_init() during early init.
//  *
//  * Assumptions:
//  *  - Kernel is 32-bit protected mode.
//  *  - You have kernel code selector 0x08 and data selector 0x10 (adjust if different).
//  *  - If you already provide gdt_set_descriptor(), it will be used. A weak fallback
//  *    is provided here that writes into an extern gdt_table[] if available.
//  *  - If you already have an 'idt' table symbol with standard entry layout, this file
//  *    will write into it. Otherwise implement the extern symbols in your IDT module.
//  */

// #include "../../../../STD/TYPEDEF.h"
// #include <string.h> /* for memset */

// /* -------------------------------------------------------------------------- */
// /* Types and configuration                                                    */
// /* -------------------------------------------------------------------------- */

// /* Packed 32-bit TSS structure (hardware format) */
// struct tss_entry {
//     U32 prev_task;
//     U32 esp0;
//     U32 ss0;
//     U32 esp1;
//     U32 ss1;
//     U32 esp2;
//     U32 ss2;
//     U32 cr3;
//     U32 eip;
//     U32 eflags;
//     U32 eax;
//     U32 ecx;
//     U32 edx;
//     U32 ebx;
//     U32 esp;
//     U32 ebp;
//     U32 esi;
//     U32 edi;
//     U32 es;
//     U32 cs;
//     U32 ss;
//     U32 ds;
//     U32 fs;
//     U32 gs;
//     U32 ldt;
//     U16 trap;
//     U16 iomap_base;
// } __attribute__((packed));

// /* IDT entry typical layout (for 32-bit gate/task gate) */
// struct idt_entry {
//     U16 offset_low;
//     U16 selector;
//     U8  zero;
//     U8  type_attr;
//     U16 offset_high;
// } __attribute__((packed));

// /* GDT entry layout used by weak setter (only for fallback) */
// struct gdt_entry {
//     U16 limit_low;
//     U16 base_low;
//     U8  base_middle;
//     U8  access;
//     U8  gran;
//     U8  base_high;
// } __attribute__((packed));

// /* -------------------------------------------------------------------------- */
// /* Configuration values - adjust if your GDT uses different selectors/indexes */
// /* -------------------------------------------------------------------------- */

// /* Kernel selectors used elsewhere in your code (common layout) */
// #define KERNEL_CODE_SELECTOR 0x08u
// #define KERNEL_DATA_SELECTOR 0x10u

// /* Choose a free GDT index for the TSS descriptor. Adjust if your GDT already uses index 5. */
// #define GDT_TSS_INDEX 5
// #define GDT_TSS_SELECTOR ((GDT_TSS_INDEX << 3) | 0x0) /* RPL 0 */

// /* Double-fault stack size (16 KB recommended but 16KB may be large; 0x4000 = 16KB) */
// #define DF_STACK_SIZE 0x4000

// /* -------------------------------------------------------------------------- */
// /* Static data                                                                 */
// /* -------------------------------------------------------------------------- */

// static struct tss_entry df_tss __attribute__((aligned(16)));
// static U8 df_stack[DF_STACK_SIZE] __attribute__((aligned(16)));

// /* -------------------------------------------------------------------------- */
// /* External hooks (optional)                                                   */
// /* If your project already provides these, the linker will prefer them.        */
// /* If not, weak fallback implementations are provided (that expect extern     */
// /* gdt_table[] and idt[] arrays).                                              */
// /* -------------------------------------------------------------------------- */

// /* Preferred API (if your GDT module already exports this, the weak fallback is ignored) */
// extern void gdt_set_descriptor(int index, U32 base, U32 limit, U8 access, U8 gran) __attribute__((weak));

// /* If your IDT table array exists, export its symbol name here as 'idt' with proper size. */
// extern struct idt_entry idt[] __attribute__((weak));

// /* If you have a gdt_table symbol, provide it (used by fallback). */
// extern struct gdt_entry gdt_table[] __attribute__((weak));

// /* -------------------------------------------------------------------------- */
// /* Weak fallback: implement gdt_set_descriptor if not provided by project      */
// /* This writes into gdt_table[] if available. If not present, fallback does   */
// /* nothing (so you must supply a gdt setter elsewhere).                        */
// /* -------------------------------------------------------------------------- */
// void gdt_set_descriptor(int index, U32 base, U32 limit, U8 access, U8 gran) {
//     /* If user provided their own, this weak will be overridden at link time. */
//     if (&gdt_table == NULL) {
//         /* no gdt_table available; do nothing */
//         return;
//     }
//     /* safety: avoid null pointer usage if gdt_table not actually provided */
//     /* write values */
//     gdt_table[index].limit_low    = (U16)(limit & 0xFFFF);
//     gdt_table[index].base_low     = (U16)(base & 0xFFFF);
//     gdt_table[index].base_middle  = (U8)((base >> 16) & 0xFF);
//     gdt_table[index].access       = access;
//     gdt_table[index].gran         = (U8)(((limit >> 16) & 0x0F) | (gran & 0xF0));
//     gdt_table[index].base_high    = (U8)((base >> 24) & 0xFF);
// }

// /* -------------------------------------------------------------------------- */
// /* idt_set_task_gate: fill IDT vector with a task gate referencing selector    */
// /* If your project already has an IDT API, prefer to call that instead.        */
// /* -------------------------------------------------------------------------- */
// void idt_set_task_gate(int vector, U16 selector) {
//     /* If idt[] not present (weak), we can't write; bail out quietly */
//     if (&idt == NULL) return;

//     /* Task gate descriptor format:
//        offset_low = 0
//        selector = TSS selector
//        zero = 0
//        type_attr = 0x85 (P=1, DPL=0, type=5 task gate)
//        offset_high = 0
//     */
//     idt[vector].offset_low = 0;
//     idt[vector].selector = selector;
//     idt[vector].zero = 0;
//     idt[vector].type_attr = 0x85; /* present, task gate */
//     idt[vector].offset_high = 0;
// }

// /* -------------------------------------------------------------------------- */
// /* df_task_entry: assembly stub executed after the CPU hardware task switch   */
// /* We write this as an inline asm function - it calls double_fault_handler()   */
// /* -------------------------------------------------------------------------- */

// /* Forward declare your C handler that already exists in ISR.c */
// extern void double_fault_handler(I32 num, U32 errcode);

// /* The function is declared naked and implemented with inline asm so no external .S needed */
// __attribute__((naked)) void df_task_entry(void) {
//     __asm__ volatile (
//         /* ensure interrupts are off and use the TSS-provided stack */
//         "cli\n\t"
//         /* push arguments for C function: errcode (0), vector (8) */
//         "pushl $0\n\t"     /* errcode */
//         "pushl $8\n\t"     /* vector */
//         "call double_fault_handler\n\t"
//         /* If handler returns, halt forever */
//         "1:\n\t"
//         "cli\n\t"
//         "hlt\n\t"
//         "jmp 1b\n\t"
//     );
// }

// /* -------------------------------------------------------------------------- */
// /* tss_init: prepare TSS struct, install TSS descriptor in GDT, LTR, and set  */
// /* IDT task gate for vector 8                                                  */
// /* -------------------------------------------------------------------------- */
// void tss_init(void) {
//     /* zero the TSS */
//     memset(&df_tss, 0, sizeof(df_tss));

//     /* Set kernel stack (ss0/esp0) in case hardware uses on privilege change */
//     df_tss.ss0 = KERNEL_DATA_SELECTOR;
//     df_tss.esp0 = (U32)&df_stack[DF_STACK_SIZE];

//     /* For the task switch we want the TSS to define the task's registers.
//        Give the TSS an initial stack and code pointer. */
//     df_tss.esp = (U32)&df_stack[DF_STACK_SIZE]; /* stack top for the new task */
//     df_tss.ss  = KERNEL_DATA_SELECTOR;

//     /* Point EIP to our df_task_entry stub so the task begins there */
//     df_tss.eip = (U32)df_task_entry;
//     df_tss.eflags = 0x202; /* IF = 1 optional - keep interrupts disabled by code */

//     /* Set code/data selectors for the new task (kernel flat) */
//     df_tss.cs = KERNEL_CODE_SELECTOR;
//     df_tss.ds = df_tss.es = df_tss.fs = df_tss.gs = KERNEL_DATA_SELECTOR;

//     /* No LDT */
//     df_tss.ldt = 0;
//     df_tss.trap = 0;

//     /* Place I/O map base at end of TSS (no I/O bitmap) */
//     df_tss.iomap_base = (U16)sizeof(df_tss);

//     /* Build TSS descriptor in GDT */
//     U32 base = (U32)&df_tss;
//     U32 limit = (U32)sizeof(df_tss) - 1;

//     /* Access byte: 0x89 => present (1), DPL 0, type 9 = 32-bit available TSS
//        Gran byte: keep 0 (byte granularity) */
//     U8 access = 0x89;
//     U8 gran = 0x00;

//     /* Install the descriptor - uses your gdt_set_descriptor if available */
//     gdt_set_descriptor(GDT_TSS_INDEX, base, limit, access, gran);

//     /* Load task register with selector for our TSS */
//     asm volatile ("ltr %0" :: "r" ((U16)GDT_TSS_SELECTOR));

//     /* Install task gate into IDT vector 8 */
//     idt_set_task_gate(8, (U16)GDT_TSS_SELECTOR);
// }

// /* -------------------------------------------------------------------------- */
// /* Optional small helper: call this after tss_init if you want to reassign   */
// /* the task gate later.                                                       */
// /* -------------------------------------------------------------------------- */
// void tss_install_double_fault_task_gate(void) {
//     idt_set_task_gate(8, (U16)GDT_TSS_SELECTOR);
// }
