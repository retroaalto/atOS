#ifndef RTOS_PROC_H
#define RTOS_PROC_H

#include <STD/TYPEDEF.h>
#include <CPU/ISR/ISR.h> // for regs struct
#include <MEMORY/PAGEFRAME/PAGEFRAME.h> // for USER_HEAP_BLOCK
#include <MEMORY/HEAP/UHEAP.h> // for USER_HEAP_BLOCK

#define USER_BINARY_VADDR 0x08048000 // same as elf loading addr
#define USER_HEAP_SIZE (4 * 1024 * 1024) // 4 MB
#define USER_STACK_SIZE (1 * 1024 * 1024) // 1 MB

// vaddr == paddr in this simple paging model
static inline ADDR virt_to_phys_pd(ADDR virt) { return (U32)(virt); }
static inline ADDR phys_to_virt_pd(ADDR phys) { return (U32)(phys); }
static inline ADDR virt_to_phys_pt(ADDR virt) { return (U32)(virt); }
static inline ADDR phys_to_virt_pt(ADDR phys) { return (U32)(phys); }

static inline ADDR proc_virt_to_phys(ADDR virt) {
    return virt - USER_BINARY_VADDR;
}
static inline ADDR proc_phys_to_virt(ADDR phys) {
    return phys + USER_BINARY_VADDR;
}

#define TCB_STATE_INACTIVE  0
#define TCB_STATE_ACTIVE    1
#define TCB_STATE_WAITING   2
#define TCB_STATE_TERMINATED 3
#define TCB_STATE_ZOMBIE    4
#define TCB_STATE_SLEEPING  5

typedef struct TrapFrame {
    // Pushed manually (segment registers)
    U32 gs;      // GS segment
    U32 fs;       // FS segment
    U32 es;        // Extra segment
    U32 ds;         // Data segment

    // Pushed by 'pushad' (note: pusha order: EAX last, EDI first)
    U32 edi;         // General purpose registers
    U32 esi;         // General purpose registers
    U32 ebp;        // Frame pointer
    U32 esp_dummy; // value at time of pusha; not used, kept for alignment
    U32 ebx;        // General purpose registers
    U32 edx;        // General purpose registers
    U32 ecx;        // General purpose registers
    U32 eax;        // General purpose registers

    // Hardware-pushed by the CPU on interrupt entry (ring0)
    U32 eip;        // Instruction pointer
    U32 cs;         // Code segment
    U32 eflags;     // CPU flags

    // No ss/esp since we stay in ring0
} TrapFrame;


typedef struct TCB {
    U32 pid;
    U32 state;
    TrapFrame *tf; // saved trap frame for context switching
    U32 *stack;
    U32 *stack_top;
    U32 *pagedir;
    USER_HEAP_BLOCK *user_heap_start;
    struct TCB *next;
} TCB;


TCB *get_master_tcb(void);
void init_multitasking(void);
void multitasking_shutdown(void);
void uninitialize_multitasking(void);
U32 get_current_pid(void);
U32 get_next_pid(void);
TCB *get_current_tcb(void);

/// @brief Run a user binary in a new process
/// @param addr Pointer to binary data
/// @param bin_size Size of binary data in bytes
/// @param heap_size Size of user heap in bytes
/// @param stack_size Size of user stack in bytes
/// @param initial_state Initial state of the new process (e.g., TCB_STATE_ACTIVE)
/// @note The binary must be a flat binary. 
/// @note IMPORTANT: Only for kernel
void RUN_BINARY(VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size, U32 initial_state);
/// @brief Terminate a process by its PID
/// @param pid Process ID to terminate
/// @note IMPORTANT: Only for kernel
void KILL_PROCESS(U32 pid);

/// Internal functions, not for public use
void early_debug_tcb(void);

TrapFrame* pit_handler_task_control(TrapFrame* tf);

#endif // RTOS_PROC_H