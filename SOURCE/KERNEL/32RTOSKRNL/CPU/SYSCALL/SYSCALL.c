#include <STD/TYPEDEF.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/VIDEO/VBE.h>
#include <STD/ASM.h>
#include <CPU/INTERRUPTS/INTERRUPTS.h>
#include <DRIVERS/PS2/KEYBOARD.h>
#include <HEAP/KHEAP.h>
#include <STD/MEM.h>
#define SYSCALL_ENTRY(id, fn) [id] = fn,
static SYSCALL_HANDLER syscall_table[SYSCALL_MAX] = {
    #include <CPU/SYSCALL/SYSCALL_LIST.h>
};
#undef SYSCALL_ENTRY

U32 SYS_PS2_KEYBOARD_RESET(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    return PS2_KEYBOARD_RESET();
}
U32 SYS_GET_CURRENT_KEY_PRESSED(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    KEYPRESS *key = (KEYPRESS *)KMALLOC(sizeof(KEYPRESS));
    KEYPRESS retval = GET_CURRENT_KEY_PRESSED();
    MEMCPY(key, &retval, sizeof(KEYPRESS));
    return (U32)key;
}
U32 SYS_GET_LAST_KEY_PRESSED(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    return (U32)(GET_LAST_KEY_PRESSED());
}
U32 SYS_KEYPRESS_TO_CHARS(U32 pointer, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    // TODO: we must calculate physical address of pointer
    // and then we can use it safely
    KEYPRESS kp;
    MEMCPY(&kp, (KEYPRESS *)pointer, sizeof(KEYPRESS));
    return (U32)KEYPRESS_TO_CHARS(&kp);
}





U32 SYS_KMALLOC(U32 size, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    return (U32)KMALLOC(size);
}
U32 SYS_KFREE(U32 pointer, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    KFREE((void *)pointer);
    return 0;
}
U32 SYS_KREALLOC(U32 pointer, U32 oldSize, U32 newSize, U32 unused4, U32 unused5) {
    (void)unused4; (void)unused5;
    return (U32)KREALLOC((void *)pointer, oldSize, newSize);
}
U32 SYS_KCALLOC(U32 num, U32 size, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused3; (void)unused4; (void)unused5;
    return (U32)KCALLOC(num, size);
}

U32 SYS_UPDATE_VRAM(U32 unused1, U32 unused2, U32 unused3, U32 unused4, U32 unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    VBE_UPDATE_VRAM();
    return 0;
}




// U32 SYS_MEMCPY(U32 dest, U32 src, U32 size, U32 unused4, U32 unused5) {
//     (void)unused4; (void)unused5;
//     MEMCPY((U0 *)dest, (CONST U0 *)src, size);
//     return dest;
// }
// U32 SYS_MEMSET(U32 dest, U8 value, U32 size, U32 unused4, U32 unused5) {
//     (void)unused4; (void)unused5;
//     MEMSET((U0 *)dest, value, size);
//     return dest;
// }







U32 syscall_dispatcher(U32 num, U32 a1, U32 a2, U32 a3, U32 a4, U32 a5) {
    if (num >= SYSCALL_MAX) return (U32)-1;

    SYSCALL_HANDLER h = syscall_table[num];
    if (!h) return (U32)-1;

    return h(a1, a2, a3, a4, a5);
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


