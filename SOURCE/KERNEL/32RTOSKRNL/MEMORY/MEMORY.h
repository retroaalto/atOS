// MEMORY.h - Master memory include file for 32RTOSKRNL
#ifndef MEMORY_H
#define MEMORY_H

#warning "TODO: PROCBIN_VADDR and __PROCESS__"

// Low Memory / BIOS areas
#define MEM_LOW_RESERVED_BASE      0x00000000
#define MEM_LOW_RESERVED_END       0x00000FFF

#define MEM_E820_BASE              0x00006000
#define MEM_E820_END               0x00006384

// VESA / VBE Info Structures
#define MEM_VESA_BASE              0x00006400
#define MEM_VESA_END               0x00007000

// Kernel entry point
#define MEM_KRNL_ENTRY_BASE        0x00100000
#define MEM_KRNL_ENTRY_END         0x00112000

// Main RTOS Kernel
#define MEM_RTOSKRNL_BASE          0x00100000
#define MEM_RTOSKRNL_END           0x00550000

// RTOS Kernel HEAP
// Leave 1 page (4 KiB) guard between heap end and stack base
#define MEM_RTOSKRNL_HEAP_AREA_BASE   0x00550000
#define MEM_RTOSKRNL_HEAP_AREA_END    0x00F33000   // just before guard page

// Derived
#define MEM_KERNEL_HEAP_BASE   MEM_RTOSKRNL_HEAP_AREA_BASE
#define MEM_KERNEL_HEAP_END    MEM_RTOSKRNL_HEAP_AREA_END

// Stack 0 (kernel stack)
// Leave 1 page guard below and 1 page guard above
#define STACK_0_GUARD_BELOW_BASE   0x00F33000
#define STACK_0_GUARD_BELOW_END    0x00F34000

#define STACK_0_BASE               0x00F34000
#define STACK_0_END                0x00F44000

#define STACK_0_GUARD_ABOVE_BASE   0x00F44000
#define STACK_0_GUARD_ABOVE_END    0x00F45000

// Framebuffer (shifted by +1 page to keep a guard above stack)
#define MEM_FRAMEBUFFER_BASE       0x00F45000  // was 0x00F44000
#define MEM_FRAMEBUFFER_END        0x013004F0

// Reserved MMIO / firmware
#define MEM_RESERVED_BASE          0x01344000
#define MEM_RESERVED_END           0x10000000  // 16 MiB

// BIOS Reserved
#define MEM_BIOS_BASE              0x000E0000
#define MEM_BIOS_END               0x00100000

// User Space

//0x02400000
#define MEM_USER_SPACE_BASE        0x10000000
#define MEM_USER_SPACE_END_MIN     0x20000000

#define MAX_ALLOWED_MEMORY         0x1FEFE0000

#endif // MEMORY_H
