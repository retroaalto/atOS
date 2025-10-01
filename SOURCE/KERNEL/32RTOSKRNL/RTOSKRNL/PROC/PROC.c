// README: Please see top of MEMORY/PAGING/PAGING.c for paging overview
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <STD/ASM.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <STD/STRING.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <STD/MEM.h>
#include <ERROR/ERROR.h>
#include <CPU/PIT/PIT.h>
#include <PROC/PROC.h> 
#include <CPU/ISR/ISR.h> // for regs struct
#define EFLAGS_IF 0x0200
#define KDS 0x10
#define KCS 0x08

static TCB master_tcb __attribute__((section(".data"))) = {0};
static TCB *current_tcb __attribute__((section(".data"))) = &master_tcb;
static U32 next_pid __attribute__((section(".data"))) = 1; // start from 1 since 0 is master tcb
static U8 initialized __attribute__((section(".data"))) = FALSE;
static U32 active_tasks __attribute__((section(".data"))) = 0;

static inline U32 PROC_READ_ESP(void) {
    U32 esp;
    ASM_VOLATILE("mov %%esp, %0" : "=r"(esp));
    return esp;
}
static inline U32 PROC_READ_EBP(void) {
    U32 ebp;
    ASM_VOLATILE("mov %%ebp, %0" : "=r"(ebp));
    return ebp;
}
static inline U32 PROC_GET_EIP(void) {
    U32 eip;
    ASM_VOLATILE("call 1f; 1: pop %0" : "=r"(eip));
    return eip;
}
static inline void PROC_SET_ESP(U32 esp) {
    ASM_VOLATILE("mov %0, %%esp" :: "r"(esp));
}
static inline void PROC_SET_EBP(U32 ebp) {
    ASM_VOLATILE("mov %0, %%ebp" :: "r"(ebp));
}
static inline void PROC_SET_EIP(U32 eip) {
    ASM_VOLATILE("jmp *%0" :: "r"(eip) : "memory");
}
static inline U32 read_cr3(void) {
    U32 val;
    ASM_VOLATILE("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3(U32 val) {
    ASM_VOLATILE("mov %0, %%cr3" :: "r"(val) : "memory");
}

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


void init_task_context(TCB *tcb, void (*entry)(void)) {
    U8 *sp = (U8 *)tcb->stack_top;
    TrapFrame *tf = (TrapFrame *)(sp - sizeof(TrapFrame));

    tf->gs = KDS; tf->fs = KDS; tf->es = KDS; tf->ds = KDS;

    tf->edi = 0; tf->esi = 0; tf->ebp = 0; tf->esp_dummy = 0;
    tf->ebx = 0; tf->edx = 0; tf->ecx = 0; tf->eax = 0;

    tf->eip = (U32)entry;
    tf->cs = KCS;
    tf->eflags = EFLAGS_IF;

    tcb->tf = tf;
    tcb->state = TCB_STATE_ACTIVE;
}

void init_master_tcb(void) {
    master_tcb.pid   = 0;
    master_tcb.state = TCB_STATE_ACTIVE;   // running
    master_tcb.next  = &master_tcb;        // circular list

    master_tcb.stack      = NULL;          // not used for master
    master_tcb.stack_top  = NULL;
    master_tcb.tf         = NULL;          // will be filled in by ISR on first tick
    master_tcb.pagedir    = get_page_directory();
    master_tcb.user_heap_start = NULL;

    active_tasks = 1;
}



void uninitialize_multitasking(void) {
    initialized = FALSE;
}

void multitasking_shutdown(void) {
    if (!initialized) return;

    // Stop preemption
    ASM_VOLATILE("cli");

    // Return to master address space
    current_tcb = &master_tcb;
    write_cr3((U32)master_tcb.pagedir);

    // Optional: force resume of master tf if needed (usually already current)
    // Leave ISR-driven restore to pick master’s tf on next interrupt, or
    // explicitly clear initialized to prevent preemption.
    initialized = FALSE;

    ASM_VOLATILE("sti");  // if you want interrupts back without scheduling
}

void init_multitasking(void) {
    if (initialized) return;
    init_master_tcb();
    PIT_INIT();

    // Simple allocation test
    VOIDPTR page = KREQUEST_PAGE();
    VOIDPTR addr = phys_to_virt_pd(page);
    panic_if(!addr, PANIC_TEXT("PANIC: Unable to request page!"), PANIC_OUT_OF_MEMORY);
    MEMZERO(addr, PAGE_SIZE);
    KFREE_PAGE(page);

    // current_tcb starts on master
    current_tcb = &master_tcb;

    // Now preemption can commence
    initialized = TRUE;
}



typedef struct {
    U32 bin_base, bin_end;
    U32 heap_base, heap_end;
    U32 stack_base, stack_end;
} user_layout_t;

static inline U32 round_up_pages(U32 bytes) {
    U32 pages = pages_from_bytes(bytes);
    return pages * PAGE_SIZE;
}

BOOLEAN compute_user_layout(U32 binary_size, U32 heap_size, U32 stack_size, user_layout_t *out) {
    if (!out) return FALSE;

    // Align all regions to page boundaries
    U32 bin_len   = round_up_pages(binary_size);
    U32 heap_len  = round_up_pages(heap_size);
    U32 stack_len = round_up_pages(stack_size);

    // Basic sanity
    if (bin_len == 0 || heap_len == 0 || stack_len == 0) return FALSE;

    // Compute bases
    U32 bin_base   = USER_BINARY_VADDR;
    U32 heap_base  = bin_base + bin_len;
    U32 stack_base = heap_base + heap_len;

    // Check overflow/wrap and kernel boundary (optional)
    // Ensure no wrap-around
    if (heap_base < bin_base) return FALSE;
    if (stack_base < heap_base) return FALSE;

    // Optional: ensure stack_end stays below kernel PDE start (keeps user in lower 3GB)
    U32 stack_end = stack_base + stack_len;
    if (stack_end < stack_base) return FALSE; // overflow
    U32 kernel_base_vaddr = (KERNEL_PDE_START << 22); // PDE index to vaddr
    if (stack_end >= kernel_base_vaddr) return FALSE;

    out->bin_base   = bin_base;
    out->bin_end    = bin_base + bin_len;
    out->heap_base  = heap_base;
    out->heap_end   = heap_base + heap_len;
    out->stack_base = stack_base;
    out->stack_end  = stack_base + stack_len;
    return TRUE;
}


static BOOLEAN map_user_binary_at(U32 *pd, U32 vbase, U8 *binary_data, U32 binary_size) {
    U32 page_count = pages_from_bytes(binary_size);
    for (U32 i = 0; i < page_count; i++) {
        VOIDPTR phys = KREQUEST_PAGE();
        if (!phys) return FALSE;
        U32 vaddr = vbase + i * PAGE_SIZE;
        map_page(pd, vaddr, (U32)phys, PAGE_PRW);

        U8 *dest = phys_to_virt_pt((U32)phys); // kernel can write via identity map
        U32 bytes = (i == page_count - 1) ? (binary_size % PAGE_SIZE) : PAGE_SIZE;
        if (bytes == 0) bytes = PAGE_SIZE;
        MEMCPY(dest, binary_data + i * PAGE_SIZE, bytes);
    }
    return TRUE;
}

static BOOLEAN map_user_heap_at(U32 *pd, U32 vbase, U32 heap_size) {
    U32 heap_pages = pages_from_bytes(heap_size);
    for (U32 i = 0; i < heap_pages; i++) {
        VOIDPTR phys = KREQUEST_PAGE();
        if (!phys) return FALSE;
        U32 vaddr = vbase + i * PAGE_SIZE;
        map_page(pd, vaddr, (U32)phys, PAGE_PRW);
        MEMZERO(phys_to_virt_pt((U32)phys), PAGE_SIZE);
    }
    return TRUE;
}

static BOOLEAN map_user_stack_at(U32 *pd, U32 vbase, U32 stack_size, TCB *proc) {
    U32 stack_pages = pages_from_bytes(stack_size);
    proc->stack = (U32 *)vbase;
    for (U32 i = 0; i < stack_pages; i++) {
        VOIDPTR phys = KREQUEST_PAGE();
        if (!phys) return FALSE;
        U32 vaddr = vbase + i * PAGE_SIZE;
        map_page(pd, vaddr, (U32)phys, PAGE_PRW);
        MEMZERO(phys_to_virt_pt((U32)phys), PAGE_SIZE);
    }
    proc->stack_top = (U32 *)(vbase + stack_pages * PAGE_SIZE);
    return TRUE;
}

PU32 create_process_pagedir(void) {
    // get a free page for the new page directory
    PU32 new_pd_phys = (PU32)KREQUEST_PAGE();
    if (!new_pd_phys) return NULL;
    
    // virtual pointer to write into new pd
    PU32 new_pd = phys_to_virt_pd(new_pd_phys);
    if (!new_pd) {
        // If you can't obtain a writable mapping for the new page, free and fail.
        KFREE_PAGE(new_pd_phys);
        return NULL;
    }
    
    // Clear the new page directory
    MEMZERO(new_pd, PAGE_SIZE);
    
    // copy kernel-side PDEs from current page directory so kernel mappings remain available.
    //    Read current CR3 to get current page directory physical address.
    PU32 cur_pd_phys = (PU32)read_cr3();
    if (!cur_pd_phys) {
        // can't read current CR3, free and fail
        KFREE_PAGE(new_pd_phys);
        return NULL;
    }
    
    PU32 cur_pd = phys_to_virt_pd(cur_pd_phys);
    if (!cur_pd) {
        // can't access current PD; cleanup and fail
        KFREE_PAGE(new_pd_phys);
        return NULL;
    }
    
    // Copy kernel PDEs (from KERNEL_PDE_START..1023).
    // Keep the exact flags as present in the current PD.
    for (U32 i = KERNEL_PDE_START; i <= KERNEL_PDE_END; ++i) {
        new_pd[i] = cur_pd[i];
        panic_debug("Created new process pagedir", 0); // Not reached at all
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
                KFREE_PAGE((VOIDPTR)page_phys);
            }
        }

        // Free the page table itself
        KFREE_PAGE((VOIDPTR)pt);
    }

    // Free the page directory itself
    KFREE_PAGE((VOIDPTR)pd);
}



void add_tcb_to_scheduler(TCB *new_tcb, U32 initial_state) {
    if (!new_tcb) return;

    new_tcb->state = initial_state;
    if (new_tcb->state == TCB_STATE_ACTIVE) {
        active_tasks++;
    }

    // Insert after master into the circular list
    TCB *master = &master_tcb;

    if (master->next == master) {
        // Only master exists
        master->next = new_tcb;
        new_tcb->next = master;
        return;
    }

    // Find last node (whose next is master)
    TCB *last = master;
    while (last->next != master) last = last->next;

    last->next = new_tcb;
    new_tcb->next = master;
}



__attribute__((noreturn))
void user_trampoline(void (*entry)(void)) {
    asm volatile(
        "call *%0\n\t"
        :
        : "r"(entry)
        : "memory"
    );
    __builtin_unreachable();
}

U32 *setup_user_process(TCB *proc, U8 *binary_data, U32 binary_size, U32 user_heap_size, U32 user_stack_size) {
    U32 *pd = create_process_pagedir();
    if (!pd) return NULL;

    proc->pagedir = pd;

    user_layout_t layout;
    if (!compute_user_layout(binary_size, user_heap_size, user_stack_size, &layout)) {
        destroy_process_pagedir(pd);
        proc->pagedir = NULL;
        return NULL;
    }
    // Map binary
    if (!map_user_binary_at(pd, layout.bin_base, binary_data, binary_size)) {
        destroy_process_pagedir(pd);
        proc->pagedir = NULL;
        return NULL;
    }

    // Map heap and remember its start
    if (!map_user_heap_at(pd, layout.heap_base, user_heap_size)) {
        destroy_process_pagedir(pd);
        proc->pagedir = NULL;
        return NULL;
    }
    proc->user_heap_start = (USER_HEAP_BLOCK *)layout.heap_base;

    // Map stack
    if (!map_user_stack_at(pd, layout.stack_base, user_stack_size, proc)) {
        destroy_process_pagedir(pd);
        proc->pagedir = NULL;
        return NULL;
    }

    init_task_context(proc, (void (*)(void))layout.bin_base);

    return pd;
}



void RUN_BINARY(VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size, U32 initial_state) {
    CLI;
    TCB *new_proc = KMALLOC(sizeof(TCB));
    panic_if(!new_proc, "Unable to allocate memory for TCB!", PANIC_OUT_OF_MEMORY);
    MEMZERO(new_proc, sizeof(TCB));

    // Build PD, map binary/heap/stack, fill TCB fields like pagedir, stack, stack_top
    panic_if(!setup_user_process(new_proc, (U8 *)file, bin_size, heap_size, stack_size),
             PANIC_TEXT("Failed to set up user process"), PANIC_OUT_OF_MEMORY);
    
    // U32 entry_vaddr = USER_BINARY_VADDR;

    // new_proc->pid = get_next_pid();

    // add_tcb_to_scheduler(new_proc, initial_state);
    STI;
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
                KFREE_PAGE(curr->stack);
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


TrapFrame* pit_handler_task_control(TrapFrame *cur) {
    // No preemption or only one task: continue current
    if (!initialized || active_tasks < 2 || current_tcb == NULL) {
        return cur;
    }

    // Save current frame
    current_tcb->tf = cur;

    // Round-robin pick
    TCB *start = (TCB *)current_tcb;
    TCB *next = start->next;
    while (next != start && next->state != TCB_STATE_ACTIVE) {
        next = next->next;
    }

    // If none runnable, continue current
    if (next == start && start->state != TCB_STATE_ACTIVE) {
        return cur;
    }

    // Address space switch if needed
    if (next->pagedir && start->pagedir != next->pagedir) {
        write_cr3((U32)next->pagedir);
    }

    // Commit
    current_tcb = next;

    // Must have a valid saved frame
    return next->tf ? next->tf : cur;
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


        ITOA(t->tf->ebp, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "EBP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->tf->eip, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "EIP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);
        VBE_UPDATE_VRAM();
    } while((t = t->next) != get_master_tcb());
}
