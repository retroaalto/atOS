#ifndef RTOSKRNL_INTERNAL_H
#define RTOSKRNL_INTERNAL_H
#include <STD/TYPEDEF.h>
#include <CPU/ISR/ISR.h> // for regs struct

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
void DUMP_STACK(U32 esp, U32 count);
void DUMP_REGS(regs *r);
void DUMP_ERRCODE(U32 errcode);
void DUMP_INTNO(U32 int_no);
void DUMP_MEMORY(U32 addr, U32 length);

void LOAD_AND_RUN_KERNEL_SHELL(VOID);



typedef struct {
    U32 *stack;        // top of process stack
    U32 esp;           // stack pointer
    U32 eip;           // instruction pointer
    U32 ebp;           // base pointer
    U32 pid;           // process ID
    U8  state;         // running, ready, blocked
    struct tcb *next;  // linked list pointer
} TCB; // Task Control Block

TCB *get_master_tcb(void);
void init_multitasking(void);
void init_master_tcb(void);
U32 get_current_pid(void);
U32 get_next_pid(void);

/// @brief Initialize a new process
/// @param tcb Pointer to last TCB
/// @param entry_point Pointer to entry function
/// @param stack_size Size of stack in pages
void init_process(TCB *tcb, U32 (*entry_point)(void), U32 stack_size);


void context_switch(void);
void schedule_next_task(void);
void pit_handler_task_control(void);

#endif // RTOSKRNL_INTERNAL_H