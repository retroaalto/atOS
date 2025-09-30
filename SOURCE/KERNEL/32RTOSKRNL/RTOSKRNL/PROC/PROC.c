#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <STD/ASM.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <STD/STRING.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <DRIVERS/PIT/PIT.h>
#include <PROC/PROC.h> 
#include <CPU/ISR/ISR.h> // for regs struct

#define USER_BINARY_VADDR 0x00000000
#define USER_HEAP_SIZE (4 * 1024 * 1024) // 4 MB
#define USER_STACK_SIZE (1 * 1024 * 1024) // 1 MB


BOOLEAN map_user_binary(U32 *pd, U8 *binary_data, U32 binary_size) {
    return TRUE;
}
BOOLEAN map_user_heap(U32 *pd, U32 binary_size, U32 heap_size) {
    return TRUE;
}
BOOLEAN map_user_stack(U32 *pd, U32 binary_size, U32 heap_size, U32 stack_size, TCB *proc) {
    return TRUE;
}

PU32 create_process_pagedir(void) {
    U32 *new_pd = (U32 *)REQUEST_PAGE();
    
    return new_pd;
}

static TCB master_tcb __attribute__((section(".data"))) = {0};
static TCB *current_tcb __attribute__((section(".data"))) = &master_tcb;
static U32 next_pid __attribute__((section(".data"))) = 1; // start from 1 since 0 is master tcb
static U8 initialized __attribute__((section(".data"))) = FALSE;

TCB *get_current_tcb(void) {
    return current_tcb;
}
TCB *get_master_tcb(void) {
    return &master_tcb;
}
U32 get_current_pid(void) {
    return master_tcb.pid;
}
U32 get_next_pid(void) {
    // TODO_FIXME: handle pid wrap-around and reuse of freed pids, though not critical now
    if(next_pid == 0xFFFFFFFF) next_pid = 2; // wrap around, avoid 0. 1 is usually SHELL
    return next_pid++;
}

static inline U32 ASM_READ_ESP(void) {
    U32 esp;
    ASM_VOLATILE("mov %%esp, %0" : "=r"(esp));
    return esp;
}

static inline U32 ASM_READ_EBP(void) {
    U32 ebp;
    ASM_VOLATILE("mov %%ebp, %0" : "=r"(ebp));
    return ebp;
}

static inline U32 ASM_GET_EIP(void) {
    U32 eip;
    ASM_VOLATILE("call 1f; 1: pop %0" : "=r"(eip));
    return eip;
}

static inline void ASM_SET_ESP(U32 esp) {
    ASM_VOLATILE("mov %0, %%esp" :: "r"(esp));
}
static inline void ASM_SET_EBP(U32 ebp) {
    ASM_VOLATILE("mov %0, %%ebp" :: "r"(ebp));
}

static inline U32 read_cr3(void) {
    U32 val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3(U32 val) {
    asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
}

void add_tcb_to_scheduler(TCB *new_tcb) {
    TCB *master = get_master_tcb();
    TCB *last = master;
    while(last->next != master) last = last->next;

    last->next = new_tcb;
    new_tcb->next = master;
    new_tcb->state = TCB_STATE_ACTIVE;
}


void init_master_tcb(void) {
    master_tcb.pid = 0;
    master_tcb.state = TCB_STATE_ACTIVE; // running
    master_tcb.next = &master_tcb; // circular list
    ASM_VOLATILE(
        "mov %%esp, %0\n\t"
        "mov %%ebp, %1\n\t"
        : "=r"(master_tcb.esp), "=r"(master_tcb.ebp)
    );
    master_tcb.stack = (U32*)master_tcb.esp;
    master_tcb.eip = 0; // not used
}


void init_process(TCB *proc, U32 (*entry_point)(void), U32 stack_size) {
    /* proc->stack should be the virtual base (lowest virtual addr of stack region).
       For downward-growing stack initial ESP should be base + stack_size_in_bytes. */
    U32 stack_pages = pages_from_bytes(stack_size);
    U32 top_of_stack = (U32)proc->stack + stack_pages * PAGE_SIZE;

    /* push initial frame as if coming from an interrupt/iret */
    U32 *sp = (U32 *)top_of_stack;
    *(--sp) = 0x200;                // initial EFLAGS
    *(--sp) = 0x1B;                 // SS (user data selector with RPL=3) if using rings
    *(--sp) = top_of_stack;         // initial ESP (user)
    *(--sp) = 0x23;                 // CS (user code selector with RPL=3)
    *(--sp) = (U32)entry_point;     // EIP

    proc->esp = (U32)sp;            // initial kernel-visible saved ESP for later swapping
    proc->eip = (U32)entry_point;
}


U32 *setup_user_process(TCB *proc, U8 *binary_data, U32 binary_size, U32 user_heap_size, U32 user_stack_size) {
    U32 *pd = create_process_pagedir();
    if (!pd) return NULL;
    proc->pagedir = pd;
    
    /* map and copy binary */
    map_user_binary(pd, binary_data, binary_size);

    /* map heap and init header in physical page */
    map_user_heap(pd, binary_size, user_heap_size);

    /* compute virtual heap start and store it in TCB (virtual addr) */
    U32 binary_pages = pages_from_bytes(binary_size);
    U32 heap_vbase = USER_BINARY_VADDR + binary_pages * PAGE_SIZE;
    proc->user_heap_start = (USER_HEAP_BLOCK *)heap_vbase;

    /* map stack and set proc->stack (virtual base) */
    map_user_stack(pd, binary_size, user_heap_size, user_stack_size, proc);

    return pd;
}





void schedule_next_task(void) {
    if(current_tcb->next == &master_tcb) {
        // Only master tcb exists
        return;
    }
    current_tcb = current_tcb->next;
    while(current_tcb->state != TCB_STATE_ACTIVE) {
        current_tcb = current_tcb->next;
        if(current_tcb == &master_tcb) {
            // No ready tasks, stay in master tcb
            current_tcb = &master_tcb;
            return;
        }
    }
}

void context_switch(TCB *next) {
    if(!next) {
        panic(PANIC_TEXT("context_switch called with NULL next TCB!"), PANIC_CONTEXT_SWITCH_NULL_TCB);
    };
    if(next == &master_tcb || next == current_tcb || !next) {
        return;
    }
    current_tcb->esp = ASM_READ_ESP();
    current_tcb->ebp = ASM_READ_EBP();

    // Switch page directory
    write_cr3((U32)next->pagedir);

    // Switch stack pointers
    ASM_SET_ESP(next->esp);
    ASM_SET_EBP(next->ebp);

    // Update current TCB
    current_tcb = next;
}



void pit_handler_task_control(void) {
    if(!initialized) return;
    TCB *prev = current_tcb;
    schedule_next_task();
    if(current_tcb != prev) {
        context_switch(current_tcb);
    }
}

void uninitialize_multitasking(void) {
    initialized = FALSE;
}

void multitasking_shutdown(void) {
    if(!initialized) return;
    current_tcb = &master_tcb;
    write_cr3((U32)master_tcb.pagedir);
    ASM_SET_ESP(master_tcb.esp);
    ASM_SET_EBP(master_tcb.ebp);
    initialized = FALSE;
}

void init_multitasking(void) {
    if(initialized) return;
    init_master_tcb();
    PIT_INIT();
    
    // Test requesting and freeing a page, should not panic
    VOIDPTR page = REQUEST_PAGE();
    panic_if(!page, PANIC_TEXT("PANIC: Unable to request page!"), PANIC_OUT_OF_MEMORY);
    FREE_PAGE(page);
    initialized = TRUE;
    return;
}

// void KILL_PROCESS(U32 pid) {
//     TCB *master = get_master_tcb();
//     TCB *prev = master;
//     TCB *curr = master->next;

//     while(curr != master) {
//         if(curr->pid == pid) {
//             // Found the process to kill
//             prev->next = curr->next; // remove from list

//             // Free resources
//             if(curr->stack) {
//                 FREE_PAGE(curr->stack);
//             }
//             if(curr->pagedir) {
//                 // Free page directory and tables (not implemented here)
//                 // You would need to walk the page directory and free all user pages and tables
//                 destroy_process_pagedir(curr->pagedir);
//             }
//             KFREE(curr);
//             return;
//         }
//         prev = curr;
//         curr = curr->next;
//     }
// }

// void RUN_BINARY(VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size) {
//     TCB *new_proc = KMALLOC(sizeof(TCB));
//     MEMZERO(new_proc, sizeof(TCB));

//     // 1. Setup process virtual memory + heap
//     setup_user_process(new_proc, (U8 *)file, bin_size, heap_size, stack_size);

//     // 2. Initialize ESP/EIP for user process
//     init_process(new_proc, (U32 (*)(void))file, stack_size);
//     new_proc->pid = get_next_pid();

//     // 3. Insert into circular linked list
//     add_tcb_to_scheduler(new_proc);
// }