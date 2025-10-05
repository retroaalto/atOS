/*
Process management and multitasking for atOS-RT.

Basically, this is kernel-level user process management with context switching
*/
#ifndef RTOS_PROC_H
#define RTOS_PROC_H

#include <STD/TYPEDEF.h>
#include <CPU/ISR/ISR.h> // for regs struct
#include <MEMORY/PAGEFRAME/PAGEFRAME.h> // for USER_HEAP_BLOCK
#include <MEMORY/HEAP/UHEAP.h> // for USER_HEAP_BLOCK
#include <MEMORY/MEMORY.h>

#define USER_BINARY_VADDR MEM_USER_SPACE_BASE // same as elf loading addr
#define USER_HEAP_SIZE (4 * 1024 * 1024) // 4 MB
#define USER_STACK_SIZE (1 * 1024 * 1024) // 1 MB
#define MAX_USER_BINARY_SIZE (16 * 1024 * 1024) // 16 MB max binary size
#define MAX_USER_MEM_SIZE MEM_USER_SPACE_END_MIN

// vaddr == paddr in this simple paging model
#define virt_to_phys_pd(virt) (U32)(virt)
#define phys_to_virt_pd(phys) (U32)(phys)
#define virt_to_phys_pt(virt) (U32)(virt)
#define phys_to_virt_pt(phys) (U32)(phys)

#define proc_virt_to_phys(virt) (virt - USER_BINARY_VADDR)
#define proc_phys_to_virt(phys) (phys + USER_BINARY_VADDR)

#define TCB_STATE_INACTIVE  0
#define TCB_STATE_ACTIVE    1
#define TCB_STATE_WAITING   2
#define TCB_STATE_TERMINATED 3
#define TCB_STATE_ZOMBIE    4
#define TCB_STATE_SLEEPING  5

/*
Trap frame is pushed automatically by the PIT interrupt handler and used for context switching.

Push order
    CPU pushes:
        eip, cs, eflags
    We push:
        gs, fs, es, ds
        edi, esi, ebp, esp (dummy), ebx, edx, ecx, eax
    (no ss/esp since we stay in ring0)

*/
#define SIZE_OF_INTERRUPT_PUSHED_REGS (sizeof(U32) * 12) // full push - eip, cs, eflags
typedef struct GeneralRegs {
    U32 eax, ecx, edx, ebx;
    U32 esp, ebp, esi, edi;
} __attribute__((packed)) GeneralRegs;

typedef struct SegmentRegs {
    U32 ds, es, fs, gs;
} __attribute__((packed)) SegmentRegs;

typedef struct CPURegs {
    U32 eip, cs, eflags;
} __attribute__((packed)) CPURegs;

typedef struct TrapFrame {
    // Manually pushed
    SegmentRegs seg;
    GeneralRegs gpr;

    // CPU interrupt auto-pushed
    CPURegs cpu;
} __attribute__((packed)) TrapFrame;


#define TASK_NAME_MAX_LEN 32
typedef struct TaskInfo {
    U32 pid;
    U8 name[TASK_NAME_MAX_LEN]; // Required for ps command
    U32 state;
    U32 cpu_time; // in ticks, total CPU time used
    U32 num_switches; // number of times scheduled
} __attribute__((packed)) TaskInfo;



typedef struct TCB {
    TaskInfo info;
    TrapFrame *tf; // saved trap frame for context switching
    U32 *stack_phys_base; // Saved as physical address
    U32 *stack_vtop; // Top of the stack as virtual address for this process

    // Memory management
    // User page directories
    U32 *pagedir; // Virtual address of page directory
    U32 *pagedir_phys; // Physical address of page directory

    VOIDPTR pages; // Pointer to allocated pages for this process (for freeing)
    U32 page_count; // Number of allocated pages

    U32 binary_size;
    U32 binary_pages;
    U32 heap_size;
    U32 heap_pages;
    U32 stack_size;
    U32 stack_pages;
    struct TCB *next;
} __attribute__((packed)) TCB;

U32 is_task_switch_needed(void);
TCB *get_master_tcb(void);
void init_multitasking(void);
void multitasking_shutdown(void);
void uninitialize_multitasking(void);
U32 get_current_pid(void);
U32 get_next_pid(void);
U32 get_last_pid(void);
TCB *get_current_tcb(void);

/// @brief Run a user binary in a new process
/// @param proc_name Name of the process. Max length is TASK_NAME_MAX_LEN-1.
/// @param file Pointer to binary data
/// @param bin_size Size of binary data in bytes
/// @param heap_size Size of user heap in bytes
/// @param stack_size Size of user stack in bytes
/// @param initial_state Initial state of the new process (e.g., TCB_STATE_ACTIVE)
/// @return TRUE on success, FALSE on failure
/// @note The binary must be a flat binary. 
/// @note IMPORTANT: Only for kernel
BOOLEAN RUN_BINARY(U8 *proc_name, VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size, U32 initial_state);
/// @brief Terminate a process by its PID
/// @param pid Process ID to terminate
/// @note IMPORTANT: Only for kernel
void KILL_PROCESS(U32 pid);

/// Internal functions, not for public use
/// pid == U32_MAX all processes
void early_debug_tcb(U32 pid);

TrapFrame* pit_handler_task_control(TrapFrame* tf);

#endif // RTOS_PROC_H