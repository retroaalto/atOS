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


#define TCB_STATE_INACTIVE  0
#define TCB_STATE_ACTIVE    1
#define TCB_STATE_WAITING   2
#define TCB_STATE_TERMINATED 3
#define TCB_STATE_ZOMBIE    4
#define TCB_STATE_SLEEPING  5

typedef struct TCB {
    U32 pid;
    U32 state;
    U32 esp;
    U32 ebp;
    U32 eip;
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
/// @note The binary must be a flat binary. 
/// @note IMPORTANT: Only for kernel
void RUN_BINARY(VOIDPTR addr, U32 bin_size, U32 heap_size, U32 stack_size);

/// @brief Terminate a process by its PID
/// @param pid Process ID to terminate
/// @note IMPORTANT: Only for kernel
void KILL_PROCESS(U32 pid);

/// Internal functions, not for public use
void early_debug_tcb(void);

#endif // RTOS_PROC_H