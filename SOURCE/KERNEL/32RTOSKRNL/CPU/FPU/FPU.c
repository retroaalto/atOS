#include <CPU/FPU/FPU.h>
#include <CPU/PIC/PIC.h>

void disable_fpu(void) {
    // Clear TS bit in CR0
    U32 cr0;
    ASM_VOLATILE("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 3); // clear TS
    ASM_VOLATILE("mov %0, %%cr0" :: "r"(cr0));

    PIC_Mask(13); // mask FPU IRQ
}