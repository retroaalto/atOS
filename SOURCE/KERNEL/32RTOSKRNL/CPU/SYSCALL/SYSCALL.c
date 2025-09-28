#include <STD/TYPEDEF.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <STD/ASM.h>
#include <CPU/INTERRUPTS/INTERRUPTS.h>

U32 SYS_EXIT(U32 code, U32 unused1, U32 unused2, U32 unused3, U32 unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    HLT;
    return 0;
}

U32 SYS_PUTS(U32 str_ptr, U32 unused1, U32 unused2, U32 unused3, U32 unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    if (!str_ptr) return -1;
    PUTS((U8*)str_ptr);
    return 0;
}
U32 SYS_UPDATE_VRAM(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    VBE_UPDATE_VRAM();
    return 0;
}
U32 SYS_DRAW_ELLIPSE(U32 x, U32 y, U32 a, U32 b, U32 color) {
    VBE_DRAW_ELLIPSE(x, y, a, b, color);
    return 0;
}


static SYSCALL_HANDLER syscall_table[SYSCALL_MAX] = {
    [SYSCALL_EXIT] = SYS_EXIT,
    [SYSCALL_PUTS] = SYS_PUTS,
    [SYSCALL_UPDATE_VRAM] = SYS_UPDATE_VRAM,
    [SYSCALL_DRAW_ELLIPSE] = SYS_DRAW_ELLIPSE,
};


U32 syscall_dispatcher(U32 num, U32 a1, U32 a2, U32 a3, U32 a4, U32 a5) {
    if(num >= SYSCALL_MAX) return -1;
    if(syscall_table[num]) {
        return syscall_table[num](a1,a2,a3,a4,a5);
    }
    return -1;
}


__attribute__((naked)) void isr_syscall(void) {
    asm volatile(
        "pusha\n\t"                  // save all general-purpose regs
        "movl %esp, %eax\n\t"        // save current esp
        "addl $28, %eax\n\t"         // adjust to point to EAX (first pusha register)
        "pushl %eax\n\t"             // push regs* argument
        "pushl $0\n\t"               // fake error code
        "pushl $0x80\n\t"            // vector
        "call isr_dispatch_c\n\t"
        "addl $12, %esp\n\t"         // pop arguments
        "popa\n\t"                   // restore registers
        "iret\n\t"
    );
}


