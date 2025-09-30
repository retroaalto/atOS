#ifndef RTOS_PROC_H
#define RTOS_PROC_H

#include <STD/TYPEDEF.h>
#include <CPU/ISR/ISR.h> // for regs struct
#include <MEMORY/PAGEFRAME/PAGEFRAME.h> // for USER_HEAP_BLOCK
#include <MEMORY/HEAP/UHEAP.h> // for USER_HEAP_BLOCK


#define TCB_STATE_INACTIVE  0
#define TCB_STATE_ACTIVE    1

typedef struct TCB {
    U32 pid;
    U32 state;
    U32 esp;
    U32 ebp;
    U32 eip;
    U32 *stack;
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

void RUN_BINARY(VOIDPTR addr, U32 bin_size, U32 heap_size, U32 stack_size);


#endif // RTOS_PROC_H