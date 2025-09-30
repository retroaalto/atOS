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



static TCB master_tcb __attribute__((section(".data"))) = {0};
static TCB *current_tcb __attribute__((section(".data"))) = &master_tcb;
static U32 next_pid __attribute__((section(".data"))) = 1; // start from 1 since 0 is master tcb
static U8 initialized __attribute__((section(".data"))) = FALSE;
static U32 active_tasks __attribute__((section(".data"))) = 0;

TCB *get_current_tcb(void) {
    return current_tcb;
}
TCB *get_master_tcb(void) {
    return &master_tcb;
}
U32 get_current_pid(void) {
    for(TCB *t = &master_tcb; ; t = t->next) {
        if(t == current_tcb) return t->pid;
        if(t->next == &master_tcb) break;
    }
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
static inline void ASM_SET_EIP(U32 eip) {
    ASM_VOLATILE("jmp *%0" :: "r"(eip) : "memory");
}
static inline U32 read_cr3(void) {
    U32 val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3(U32 val) {
    asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
}

BOOLEAN map_user_binary(U32 *pd, U8 *binary_data, U32 binary_size) {
    U32 page_count = pages_from_bytes(binary_size);
    U32 virt = USER_BINARY_VADDR;

    for (U32 i = 0; i < page_count; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) return FALSE;

        // Map page into page directory with present | RW
        map_page(pd, virt + i * PAGE_SIZE, (U32)phys, PAGE_PRW);

        // Copy binary data into newly allocated page
        U8 *dest = (U8 *)(virt + i * PAGE_SIZE); // identity-mapped virtual
        U32 bytes_to_copy = (i == page_count - 1) ? (binary_size % PAGE_SIZE) : PAGE_SIZE;
        if (bytes_to_copy == 0) bytes_to_copy = PAGE_SIZE;
        MEMCPY(dest, binary_data + i * PAGE_SIZE, bytes_to_copy);
    }

    return TRUE;
}

BOOLEAN map_user_heap(U32 *pd, U32 binary_size, U32 heap_size) {
    U32 bin_pages = pages_from_bytes(binary_size);
    U32 heap_pages = pages_from_bytes(heap_size);
    U32 virt = USER_BINARY_VADDR + bin_pages * PAGE_SIZE;

    for (U32 i = 0; i < heap_pages; i++) {
        VOIDPTR phys = REQUEST_PAGE();
        if (!phys) return FALSE;

        map_page(pd, virt + i * PAGE_SIZE, (U32)phys, PAGE_PRW);
        MEMZERO((U8 *)(virt + i * PAGE_SIZE), PAGE_SIZE);
    }

    return TRUE;
}

BOOLEAN map_user_stack(U32 *pd, U32 binary_size, U32 heap_size, U32 stack_size, TCB *proc) {
    U32 bin_pages = pages_from_bytes(binary_size);
    // U32 heap_pages = pages_from_bytes(heap_size);
    // U32 stack_pages = pages_from_bytes(stack_size);

    // Stack is placed after heap
    // U32 stack_base = USER_BINARY_VADDR + (bin_pages + heap_pages) * PAGE_SIZE;

    // Save virtual base of stack in TCB (lowest virtual address)
    // proc->stack = (U32 *)stack_base;

    // for (U32 i = 0; i < stack_pages; i++) {
    //     VOIDPTR phys = REQUEST_PAGE();
    //     if (!phys) return FALSE;

    //     map_page(pd, stack_base + i * PAGE_SIZE, (U32)phys, PAGE_PRW);
    //     MEMZERO((U8 *)(stack_base + i * PAGE_SIZE), PAGE_SIZE);
    // }

    // Save top of stack in TCB for convenience
    // proc->stack_top = (U32 *)(stack_base + stack_pages * PAGE_SIZE);

    // return TRUE;
}



PU32 create_process_pagedir(void) {
    // get a free page for the new page directory
    PU32 new_pd_phys = (PU32)REQUEST_PAGE();
    if (!new_pd_phys) return NULL;

    // virtual pointer to write into new pd
    PU32 new_pd = phys_to_virt_pd(new_pd_phys);
    if (!new_pd) {
        // If you can't obtain a writable mapping for the new page, free and fail.
        FREE_PAGE(new_pd_phys);
        return NULL;
    }

    // Clear the new page directory
    MEMZERO(new_pd, PAGE_SIZE);

    // copy kernel-side PDEs from current page directory so kernel mappings remain available.
    //    Read current CR3 to get current page directory physical address.
    PU32 cur_pd_phys = (PU32)read_cr3();
    if (!cur_pd_phys) {
        // can't read current CR3, free and fail
        FREE_PAGE(new_pd_phys);
        return NULL;
    }

    PU32 cur_pd = phys_to_virt_pd(cur_pd_phys);
    if (!cur_pd) {
        // can't access current PD; cleanup and fail
        FREE_PAGE(new_pd_phys);
        return NULL;
    }

    // Copy kernel PDEs (from KERNEL_PDE_START..1023).
    // Keep the exact flags as present in the current PD.
    for (U32 i = KERNEL_PDE_START; i < PAGE_ENTRIES; ++i) {
        new_pd[i] = cur_pd[i];
    }

    return new_pd;
}

void destroy_process_pagedir(U32 *pd) {
    if (!pd) return;

    // Loop through all user page directory entries (before kernel PDEs)
    for (U32 pd_index = 0; pd_index < KERNEL_PDE_START; ++pd_index) {
        if (!(pd[pd_index] & PAGE_PRESENT)) continue;

        // Get page table address (identity-mapped)
        U32 *pt = (U32 *)(pd[pd_index] & ~0xFFF);

        // Free all present pages in the table
        for (U32 pt_index = 0; pt_index < PAGE_ENTRIES; ++pt_index) {
            if (pt[pt_index] & PAGE_PRESENT) {
                U32 page_phys = pt[pt_index] & ~0xFFF;
                FREE_PAGE((VOIDPTR)page_phys);
            }
        }

        // Free the page table itself
        FREE_PAGE((VOIDPTR)pt);
    }

    // Free the page directory itself
    FREE_PAGE((VOIDPTR)pd);
}



void add_tcb_to_scheduler(TCB *new_tcb) {
    if (!new_tcb) return;

    // Optional: let caller decide initial state
    if (new_tcb->state == 0) {
        new_tcb->state = TCB_STATE_ACTIVE;
    }

    new_tcb->next = new_tcb; // self-loop initially

    TCB *master = &master_tcb;
    if (!master) {
        // First-ever TCB in scheduler
        master_tcb = *new_tcb;
        return;
    }

    // Insert into circular list
    if (master->next == master) {
        // Only master exists, new TCB becomes next
        master->next = new_tcb;
        new_tcb->next = master;
    }
    TCB *last = master;
    while(last->next != master) last = last->next;
    last->next = new_tcb;
    new_tcb->next = master;
}


__attribute__((noreturn))
void user_trampoline(void (*entry)(void)) {
    // Entry point is in EDI (or pushed on stack)
    asm volatile(
        "movl 4(%esp), %eax\n\t"   // get entry pointer from stack
        "jmp *%eax\n\t"            // jump to user binary
    );
    __builtin_unreachable();
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
    active_tasks = 1;
    master_tcb.pagedir = get_page_directory(); // current page directory
    master_tcb.user_heap_start = NULL; // kernel has no user heap
}


void init_process(TCB *proc, U32 entry_point, U32 stack_size) {
    U32 stack_pages = pages_from_bytes(stack_size);
    U32 top_of_stack = (U32)proc->stack_top;
    
    U32 *sp = (U32 *)top_of_stack;

    // Push entry_point as argument for trampoline
    *(--sp) = entry_point;

    // Push dummy return address for trampoline (not used)
    *(--sp) = 0;

    proc->esp = (U32)sp;
    proc->ebp = (U32)sp;

    // EIP points to trampoline function
    proc->eip = (U32)user_trampoline;
}

void INC_rki_row(U32 *rki_row) {
    *rki_row += VBE_CHAR_HEIGHT + 2;
}
void early_debug_tcb(void) {
    // for early task debugging
    TCB *t = get_master_tcb();
    U32 rki_row = 0;
    VBE_CLEAR_SCREEN(VBE_BLACK);
    VBE_DRAW_STRING(0, rki_row, "TCB Dump:", VBE_GREEN, VBE_BLACK);
    INC_rki_row(&rki_row);
    do {
        U8 buf[20];
        INC_rki_row(&rki_row);
        
        ITOA(t->pid, buf, 10);
        VBE_DRAW_STRING(0, rki_row, "TCB PID:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->state, buf, 10);
        VBE_DRAW_STRING(0, rki_row, "State:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);
        
        ITOA(t->esp, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "ESP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->ebp, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "EBP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->eip, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "EIP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);
        VBE_UPDATE_VRAM();
    } while((t = t->next) != get_master_tcb());
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
    active_tasks++;
    /* map stack and set proc->stack (virtual base) */
    map_user_stack(pd, binary_pages * PAGE_SIZE, user_heap_size, user_stack_size, proc);

    return pd;
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

void KILL_PROCESS(U32 pid) {
    TCB *master = get_master_tcb();
    TCB *prev = master;
    TCB *curr = master->next;

    while(curr != master) {
        if(curr->pid == pid) {
            // Found the process to kill
            prev->next = curr->next; // remove from list

            // Free resources
            if(curr->stack) {
                FREE_PAGE(curr->stack);
            }
            if(curr->pagedir) {
                // Free page directory and tables (not implemented here)
                // You would need to walk the page directory and free all user pages and tables
                destroy_process_pagedir(curr->pagedir);
            }
            KFREE(curr);
            return;
            active_tasks--;
        }
        prev = curr;
        curr = curr->next;
    }
}

void RUN_BINARY(VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size) {
    TCB *new_proc = KMALLOC(sizeof(TCB));
    panic_if(!new_proc, "Unable to allocate memory for TCB!", PANIC_OUT_OF_MEMORY);
    MEMZERO(new_proc, sizeof(TCB));

    // Setup memory
    setup_user_process(new_proc, (U8 *)file, bin_size, heap_size, stack_size);

    // Initialize process ESP/EIP using trampoline
    U32 entry_vaddr = USER_BINARY_VADDR; // start of user binary

    
    // init_process(new_proc, entry_vaddr, stack_size);

    // new_proc->pid = get_next_pid();

    // add_tcb_to_scheduler(new_proc);
}


void context_switch(TCB *next) {
    if (!next) {
        panic(PANIC_TEXT("context_switch called with NULL next TCB!"), PANIC_CONTEXT_SWITCH_NULL_TCB);
    }

    if (next == current_tcb) {
        if(active_tasks < 2) {
            // Only one active task, no switch needed
            return;
        }
    }
    // Save current task context
    if (current_tcb) {
        current_tcb->esp = ASM_READ_ESP();
        current_tcb->ebp = ASM_READ_EBP();
    }

    // Switch page directory only if needed
    if (next->pagedir && (!current_tcb || current_tcb->pagedir != next->pagedir)) {
        write_cr3((U32)next->pagedir);
    }

    // Restore next task context
    ASM_SET_ESP(next->esp);
    ASM_SET_EBP(next->ebp);
    // Jump to next task's EIP

    // Update current TCB
    current_tcb = next;
    // Execution will resume in next task automatically
    ASM_SET_EIP(next->eip);
}


void pit_handler_task_control(void) {
    CLI;
    if (!initialized) {
        STI;
        return;
    }

    TCB *start = current_tcb;
    do {
        current_tcb = current_tcb->next;

        if (current_tcb->state == TCB_STATE_ACTIVE) {
            if(active_tasks < 2) {
                // Only one active task, no switch needed
                current_tcb = start;
                STI;
                return;
            }
            // Found a runnable task
            context_switch(current_tcb);
            STI;
            return;
        }
    } while (current_tcb != start);
    // No runnable task found, optionally switch to idle
    current_tcb = start; // ensure we keep current task
    STI;
}

