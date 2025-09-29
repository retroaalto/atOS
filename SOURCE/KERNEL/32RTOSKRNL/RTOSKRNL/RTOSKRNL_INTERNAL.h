#ifndef RTOSKRNL_INTERNAL_H
#define RTOSKRNL_INTERNAL_H
#include <STD/TYPEDEF.h>
#include <CPU/ISR/ISR.h> // for regs struct
#include <MEMORY/PAGING/PAGEFRAME.h> // for USER_HEAP_BLOCK
typedef enum {

    PANIC_NONE = 0,
    PANIC_OUT_OF_MEMORY,
    PANIC_INVALID_PROCESS,
    PANIC_STACK_OVERFLOW,
    PANIC_INVALID_TCB,
    PANIC_CONTEXT_SWITCH_FAILED,
    PANIC_SCHEDULER_FAILED,
    PANIC_PIT_INIT_FAILED,

    PANIC_PAGE_FAULT_IN_KERNEL,

    PANIC_KERNEL_SHELL_GENERAL_FAILURE,
    PANIC_UNKNOWN_ERROR
} PANIC_CODES;

void panic(const U8 *msg, U32 errmsg);
void panic_if(BOOL condition, const U8 *msg, U32 errmsg);   
void DUMP_STACK(U32 esp, U32 count);
void DUMP_REGS(regs *r);
void DUMP_ERRCODE(U32 errcode);
void DUMP_INTNO(U32 int_no);
void DUMP_MEMORY(U32 addr, U32 length);

void LOAD_AND_RUN_KERNEL_SHELL(VOID);

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
U32 get_current_pid(void);
U32 get_next_pid(void);
TCB *get_current_tcb(void);

void RUN_BINARY(VOIDPTR addr, U32 bin_size, U32 heap_size, U32 stack_size);

#endif // RTOSKRNL_INTERNAL_H