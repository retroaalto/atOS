#include <CPU/PIT/PIT.h>
#include <CPU/PIC/PIC.h>
#include <STD/ASM.h>
#include <CPU/ISR/ISR.h>
#include <RTOSKRNL_INTERNAL.h>
#include <CPU/IDT/IDT.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

#define KCS 0x08
#define KDS 0x10
#define _STR_HELPER(x) #x
#define _STR(x) _STR_HELPER(x)

void pit_set_frequency(U32 freq) {
    U32 divisor = PIT_FREQUENCY / freq;

    // Send command byte: channel 0, access low+high, mode 3 (square wave), binary
    _outb(PIT_COMMAND, 0x36);

    // Send divisor low byte, then high byte
    _outb(PIT_CHANNEL0, (U8)(divisor & 0xFF));
    _outb(PIT_CHANNEL0, (U8)((divisor >> 8) & 0xFF));
}

static volatile U32 ticks __attribute__((section(".data"))) = 0;
static U32 hz __attribute__((section(".data"))) = 100;
static BOOLEAN initialized __attribute__((section(".data"))) = FALSE;

U0 PIT_WAIT_MS(U32 ms) {
    U32 start = ticks;
    U32 waitTicks = (hz * ms) / 1000;
    if(waitTicks == 0) waitTicks = 1; // minimum wait of 1 tick
    while((ticks - start) < waitTicks) {
        ASM_VOLATILE("hlt");
    }
}

U0 PIT_INIT(U0) {
    CLI;
    if(initialized) return;
    initialized = TRUE;
    hz = 100;
    pit_set_frequency(hz);
    idt_set_gate(PIT_VECTOR, (U32)isr_pit, KCS, 0x8E);
    ISR_REGISTER_HANDLER(PIC_REMAP_OFFSET + 0, isr_pit);
    // Unmask IRQ0 (PIT)
    PIC_Unmask(0);
    ticks = 0;
    STI;
}

U32 *PIT_GET_TICKS_PTR() {
    return &ticks;
}

U32 *PIT_GET_HZ_PTR() {
    return &hz;
}

__attribute__((naked)) void isr_pit(void) {
    asm volatile(
        // Enter with hardware frame already on stack: [EIP][CS][EFLAGS]
        "cli\n\t"

        // Save segment registers in the order TrapFrame expects (gs, fs, es, ds)
        "pushl %gs\n\t"
        "pushl %fs\n\t"
        "pushl %es\n\t"
        "pushl %ds\n\t"

        // Save general-purpose registers (edi first, eax last)
        "pushal\n\t"

        // Switch to kernel data segments while in ISR
        "movw $" _STR(KDS) ", %ax\n\t"
        "movw %ax, %ds\n\t"
        "movw %ax, %es\n\t"
        "movw %ax, %fs\n\t"
        "movw %ax, %gs\n\t"

        // Bump system tick
        "incl ticks\n\t"

        // Call scheduler with pointer to current TrapFrame (ESP points at gs field)
        "pushl %esp\n\t"
        "call pit_handler_task_control\n\t"
        "addl $4, %esp\n\t"

        // Switch to next task’s saved trap frame (returned in EAX)
        "movl %eax, %esp\n\t"

        // Send EOI to PIC (master)
        "movb $0x20, %al\n\t"
        "outb %al, $0x20\n\t"

        // Restore registers from next TrapFrame (must mirror save order)
        "popal\n\t"
        "popl %ds\n\t"
        "popl %es\n\t"
        "popl %fs\n\t"
        "popl %gs\n\t"

        // Return to next task (uses hardware frame at tail of TrapFrame)
        "iret\n\t"
    );
}
