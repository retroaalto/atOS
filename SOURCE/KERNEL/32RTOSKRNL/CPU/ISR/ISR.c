/*+++
    Source/KERNEL/32RTOSKRNL/CPU/ISR/ISR.c - Interrupt Service Routine (ISR) Implementation

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    32-bit interrupt service routine (ISR) implementation for atOS-RT.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/24 - Antonako1
        Initial version. ISR handling functions added.
REMARKS
---*/
#include "./ISR.h"
#include "../IRQ/IRQ.h"
#include "../IDT/IDT.h"
#include "../../../../STD/ASM.h"
#include "../INTERRUPTS.h"

ISRHandler g_Handlers[IDT_COUNT];

// This is the c-level exception handler
void isr_common_handler(I32 num, U32 errcode) {
    (void)errcode; (void)num;
    for (;;) 
        __asm__ volatile(
            "cli\n\t"
            "hlt\n\t"
        );
}

void irq_common_handler(I32 num, U32 errcode) {
    (void)errcode; // Not used
    if(num >= 32 && num < 48) {
        pic_send_eoi(num - 32);
    }
}


// This function is called from assembly
void isr_dispatch_c(int vector, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    // For CPU exceptions (0-31) call isr_common_handler
    if (vector < 32) {
        isr_common_handler(vector, errcode);
    } else if (vector >= 32 && vector < 48) {
        irq_common_handler(vector - 32, errcode);
    } else {
        isr_common_handler(vector, errcode);
    }
}

void irq_dispatch_c(int irq, U32 errcode, regs *regs_ptr) {
    (void)regs_ptr;
    // call irq_common_handler(irq)
    irq_common_handler(irq, errcode);
}


void ISR_REGISTER_HANDLER(U32 int_no, ISRHandler handler) {
    if(int_no < IDT_COUNT) {
        g_Handlers[int_no] = handler;
    } else {
    }
}
U0 SETUP_ISRS(U0) {
    U16 cs = 0x08; // code selector
    U8 flags = 0x8E; // present, ring0, 32-bit interrupt gate

    // CPU exceptions
    idt_set_gate(0,  (U0*)&isr0,  cs, flags);
    idt_set_gate(1,  (U0*)&isr1,  cs, flags);
    idt_set_gate(2,  (U0*)&isr2,  cs, flags);
    idt_set_gate(3,  (U0*)&isr3,  cs, flags);
    idt_set_gate(4,  (U0*)&isr4,  cs, flags);
    idt_set_gate(5,  (U0*)&isr5,  cs, flags);
    idt_set_gate(6,  (U0*)&isr6,  cs, flags);
    idt_set_gate(7,  (U0*)&isr7,  cs, flags);
    idt_set_gate(8,  (U0*)&isr8,  cs, flags);
    idt_set_gate(9,  (U0*)&isr9,  cs, flags);
    idt_set_gate(10, (U0*)&isr10, cs, flags);
    idt_set_gate(11, (U0*)&isr11, cs, flags);
    idt_set_gate(12, (U0*)&isr12, cs, flags);
    idt_set_gate(13, (U0*)&isr13, cs, flags);
    idt_set_gate(14, (U0*)&isr14, cs, flags);
    idt_set_gate(15, (U0*)&isr15, cs, flags);
    idt_set_gate(16, (U0*)&isr16, cs, flags);
    idt_set_gate(17, (U0*)&isr17, cs, flags);
    idt_set_gate(18, (U0*)&isr18, cs, flags);
    idt_set_gate(19, (U0*)&isr19, cs, flags);
    idt_set_gate(20, (U0*)&isr20, cs, flags);
    idt_set_gate(21, (U0*)&isr21, cs, flags);
    idt_set_gate(22, (U0*)&isr22, cs, flags);
    idt_set_gate(23, (U0*)&isr23, cs, flags);
    idt_set_gate(24, (U0*)&isr24, cs, flags);
    idt_set_gate(25, (U0*)&isr25, cs, flags);
    idt_set_gate(26, (U0*)&isr26, cs, flags);
    idt_set_gate(27, (U0*)&isr27, cs, flags);
    idt_set_gate(28, (U0*)&isr28, cs, flags);
    idt_set_gate(29, (U0*)&isr29, cs, flags);
    idt_set_gate(30, (U0*)&isr30, cs, flags);
    idt_set_gate(31, (U0*)&isr31, cs, flags);

    // IRQs 32..47
    idt_set_gate(32, (U0*)&irq32, cs, flags);
    idt_set_gate(33, (U0*)&irq33, cs, flags);
    idt_set_gate(34, (U0*)&irq34, cs, flags);
    idt_set_gate(35, (U0*)&irq35, cs, flags);
    idt_set_gate(36, (U0*)&irq36, cs, flags);
    idt_set_gate(37, (U0*)&irq37, cs, flags);
    idt_set_gate(38, (U0*)&irq38, cs, flags);
    idt_set_gate(39, (U0*)&irq39, cs, flags);
    idt_set_gate(40, (U0*)&irq40, cs, flags);
    idt_set_gate(41, (U0*)&irq41, cs, flags);
    idt_set_gate(42, (U0*)&irq42, cs, flags);
    idt_set_gate(43, (U0*)&irq43, cs, flags);
    idt_set_gate(44, (U0*)&irq44, cs, flags);
    idt_set_gate(45, (U0*)&irq45, cs, flags);
    idt_set_gate(46, (U0*)&irq46, cs, flags);
    idt_set_gate(47, (U0*)&irq47, cs, flags);

    for(U32 i = 48; i < IDT_COUNT; i++) {
        idt_set_gate(i, (U0*)&isr48, cs, flags); // point to isr48 for now
    }
}

VOID SETUP_ISR_HANDLERS(VOID) {    
    for(int i = 0; i < IDT_COUNT; i++) {
        if(i < 32) {
            ISR_REGISTER_HANDLER(i, isr_common_handler); // CPU exceptions
        } else if(i >= 32 && i < 48) {
            ISR_REGISTER_HANDLER(i, irq_common_handler);
        } else {
            ISR_REGISTER_HANDLER(i, isr_common_handler); // Reserved / unused
        }
    }
}

// typedef struct {
//     U32 prev_tss;
//     U32 esp0;
//     U32 ss0;
//     U32 esp1;
//     U32 ss1;
//     U32 esp2;
//     U32 ss2;
//     U32 cr3;
//     U32 eip;
//     U32 eflags;
//     U32 eax, ecx, edx, ebx, esp, ebp, esi, edi;
//     U16 es, cs, ss, ds, fs, gs;
//     U16 ldt;
//     U16 trap, iomap_base;
// } __attribute__((packed)) tss_entry_t;

// tss_entry_t tss;


// __attribute__((aligned(16))) U8 df_stack[DF_STACK_SIZE];

// void memset(void *ptr, I32 value, U32 num) {
//     UCHAR *p = ptr;
//     while (num--) {
//         *p++ = value;
//     }
// }

// void tss_init(void) {
//     memset(&tss, 0, sizeof(tss_entry_t));
//     tss.ss0  = 0x10;                          // Kernel data segment
//     tss.esp0 = (U32)&df_stack[DF_STACK_SIZE]; // Top of DF stack
//     tss.iomap_base = sizeof(tss_entry_t);
    
//     // Load TSS into GDT and LTR
//     // Make sure you have a TSS descriptor in GDT
//     asm volatile("ltr %%ax" : : "a"(0x28)); // Example selector
// }
