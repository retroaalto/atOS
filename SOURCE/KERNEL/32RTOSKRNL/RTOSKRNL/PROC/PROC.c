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
#include <CPU/PIC/PIC.h>
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
static inline U32 PROC_READ_EIP(void) {
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

U32 get_active_task_count(void) {
    return active_tasks;
}

TCB *get_current_tcb(void) {
    return current_tcb;
}
TCB *get_master_tcb(void) {
    return &master_tcb;
}
U32 get_current_pid(void) {
    for(TCB *t = &master_tcb; ; t = t->next) {
        if(t == current_tcb) return t->info.pid;
        if(t->next == &master_tcb) break;
    }
    return master_tcb.info.pid;
}
U32 get_last_pid(void) {
    return next_pid - 1;
}
U32 get_next_pid(void) {
    // TODO_FIXME: handle pid wrap-around and reuse of freed pids, though not critical now
    if(next_pid == 0xFFFFFFFF) next_pid = 2; // wrap around, avoid 0. 1 is usually SHELL
    return next_pid++;
}

static void init_master_tcb(void) {
    master_tcb.info.pid   = 0;
    master_tcb.info.state = TCB_STATE_ACTIVE;   // running
    master_tcb.info.cpu_time = 0;
    master_tcb.info.num_switches = 0;
    STRNCPY((char *)master_tcb.info.name, "32RTOSKRNL", TASK_NAME_MAX_LEN - 1);
    
    master_tcb.next  = &master_tcb;        // circular list

    master_tcb.stack_phys_base = NULL;          // not used for master
    master_tcb.stack_vtop  = NULL;
    master_tcb.tf         = NULL;          // will be filled in by ISR on first tick
    master_tcb.pagedir    = (read_cr3()  & ~0xFFF);
    master_tcb.pagedir_phys = (read_cr3()  & ~0xFFF); // current page directory
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
    STI;
    if (initialized) return;
    init_master_tcb();

    set_next_task_cr3_val((U32)master_tcb.pagedir);
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
    PIT_INIT();
}



typedef struct {
    U32 bin_base, bin_end;
    U32 heap_base, heap_end;
    U32 stack_base, stack_end;
} user_layout_t;

typedef struct {
    U32 *virt; // kernel-mapped pointer. 1:1 mapping
    U32 phys;  // physical page. Same as virt
} PD_HANDLE;

static inline U32 round_up_pages(U32 bytes) {
    U32 pages = pages_from_bytes(bytes);
    return pages * PAGE_SIZE;
}

void copy_pde_range(U32 *dest_pd, U32 *src_pd, U32 start_vaddr, U32 end_vaddr) {
    // Calculate the starting and ending PDE indices, aligned to 4MB boundaries
    U32 pde_start = start_vaddr >> 22;
    U32 pde_end = (end_vaddr + 0x3FFFFF) >> 22; // Add 4MB-1 to include the last page

    if(pde_start > pde_end || pde_end >= PAGE_ENTRIES) return; // Invalid range
    if(pde_start > USER_PDE && pde_end < USER_PDE_END) return; // Skip user PDEs 

    for (U32 i = pde_start; i <= pde_end; ++i) {
        if (src_pd[i] & PAGE_PRESENT) {
            dest_pd[i] = src_pd[i];
        }
    }
}

void copy_kernel_pdes_with_offset(U32 *new_pd, U32 *kernel_pd) {
    for(U32 i = 0; i < PAGE_ENTRIES; i++) {
        // Skip user space PDEs
        // if(i >= USER_PDE && i <= USER_PDE_END) continue;
        if(kernel_pd[i] & PAGE_PRESENT) {
            new_pd[i] = kernel_pd[i];
        }
    }

    // VBE_MODEINFO *vmi = GET_VBE_MODE();
    
    // // Copy the specific kernel PDE ranges.
    // copy_pde_range(new_pd, kernel_pd, MEM_LOW_RESERVED_BASE, MEM_LOW_RESERVED_END);
    // copy_pde_range(new_pd, kernel_pd, MEM_E820_BASE, MEM_E820_END);
    // copy_pde_range(new_pd, kernel_pd, MEM_VESA_BASE, MEM_VESA_END);
    
    // if(vmi && vmi->PhysBasePtr) {
    //     U32 fb_start = vmi->PhysBasePtr;
    //     U32 fb_size = vmi->YResolution * vmi->BytesPerScanLine;
    //     U32 fb_end = fb_start + fb_size;
    //     copy_pde_range(new_pd, kernel_pd, fb_start, fb_end);
    // }
        
    // copy_pde_range(new_pd, kernel_pd, MEM_RTOSKRNL_BASE, MEM_RTOSKRNL_END);
    // copy_pde_range(new_pd, kernel_pd, MEM_KERNEL_HEAP_BASE, MEM_KERNEL_HEAP_END);
    // copy_pde_range(new_pd, kernel_pd, STACK_0_BASE, STACK_0_END);
    // copy_pde_range(new_pd, kernel_pd, MEM_FRAMEBUFFER_BASE, MEM_FRAMEBUFFER_END);
}
    
PD_HANDLE create_process_pagedir(void) {
    VOIDPTR new_pd_phys = (VOIDPTR)KREQUEST_PAGE();
    if (!new_pd_phys) return (PD_HANDLE){0,0};

    MEMZERO(new_pd_phys, PAGE_SIZE);

    return (PD_HANDLE){ .virt = proc_phys_to_virt(new_pd_phys), .phys = (U32)new_pd_phys };
}

void destroy_process_pagedir(U32 *pd) {
    if (!pd) return;

    // Loop through all user page directory entries (before kernel PDEs)
    #warning TODO: optimize by tracking allocated pages in TCB

    // Free the page directory itself
    KFREE_PAGE((VOIDPTR)pd);
}

/*
Data is now in order:
binary
heap
stack
*/
BOOLEAN compute_user_layout(VOIDPTR phys_addr, U32 binary_size, U32 heap_size, U32 stack_size, user_layout_t *out) {
    if (!out || !phys_addr) return FALSE;

    // Align all regions to page boundaries
    U32 bin_len   = round_up_pages(binary_size);
    U32 heap_len  = round_up_pages(heap_size);
    U32 stack_len = round_up_pages(stack_size);
    
    // Basic sanity
    if (bin_len == 0 || heap_len == 0 || stack_len == 0) return FALSE;
    
    // Compute bases
    U32 bin_base   = (U32)phys_addr;
    U32 heap_base  = bin_base + bin_len;
    U32 stack_base = heap_base + heap_len;
    
    // Check overflow/wrap and kernel boundary (optional)
    // Ensure no wrap-around
    if (heap_base < bin_base) return FALSE;
    if (stack_base < heap_base) return FALSE;

    out->bin_base   = bin_base;
    out->bin_end    = bin_base + bin_len;
    panic_if(!is_page_aligned(out->bin_base) || !is_page_aligned(out->bin_end),
         PANIC_TEXT("Binary region not page-aligned"), PANIC_INVALID_ARGUMENT);
    out->heap_base  = heap_base;
    out->heap_end   = heap_base + heap_len;
    panic_if(!is_page_aligned(out->heap_base) || !is_page_aligned(out->heap_end),
         PANIC_TEXT("Heap region not page-aligned"), PANIC_INVALID_ARGUMENT);
    
    out->stack_base = stack_base;
    out->stack_end  = stack_base + stack_len;
    panic_if(!is_page_aligned(out->stack_base) || !is_page_aligned(out->stack_end),
         PANIC_TEXT("Stack region not page-aligned"), PANIC_INVALID_ARGUMENT);

    return TRUE;
}

// #define MAP_IN_FUNCTIONS

static BOOLEAN map_user_binary_at(VOIDPTR pages, U32 page_count, user_layout_t *layout,
                                  U32 *pd, U32 vbase, U8 *binary_data, U32 binary_size) {
    if (!pages || !layout || !pd) return FALSE;
    if (page_count == 0) return FALSE;
    if (layout->bin_base == 0 || layout->bin_end == 0) return FALSE;
    if (vbase == 0) return FALSE;
    if (binary_size == 0) return FALSE;
    if (binary_size > (layout->bin_end - layout->bin_base)) return FALSE;
    
    for (U32 i = 0; i < page_count; i++) {
        U32 phys = (U32)pages + (i * PAGE_SIZE);
        
        U32 offset = i * PAGE_SIZE;
        U32 copy_size = (i == page_count - 1) ? (binary_size - offset) : PAGE_SIZE;
        MEMZERO(phys, PAGE_SIZE);
        if (copy_size > 0 && binary_data) {
            MEMCPY(phys, binary_data + offset, copy_size);
        }
        #ifdef MAP_IN_FUNCTIONS
        U32 vaddr = vbase + (i * PAGE_SIZE);
        map_page(pd, vaddr, phys, PAGE_PRW);
        #endif
    }
    return TRUE;
}


static BOOLEAN map_user_heap_at(VOIDPTR pages, U32 heap_pages, user_layout_t *layout, 
                                U32 *pd, U32 vbase, U32 heap_size) {
    if (!pages || !layout || !pd) return FALSE;
    if (!vbase || heap_size == 0) return FALSE;
    if (layout->heap_base == 0 || layout->heap_end == 0) return FALSE;
    if (heap_size > (layout->heap_end - layout->heap_base)) return FALSE;

    U32 start_page = (layout->heap_base - layout->bin_base) / PAGE_SIZE;

    for (U32 i = 0; i < heap_pages; i++) {
        VOIDPTR phys = (VOIDPTR)((U32)pages + (start_page + i) * PAGE_SIZE);
        MEMZERO((U32)phys, PAGE_SIZE);
        #ifdef MAP_IN_FUNCTIONS
        U32 vaddr = vbase + i * PAGE_SIZE;
        map_page(pd, vaddr, (U32)phys, PAGE_PRW);
        #endif
    }
    return TRUE;
}

static BOOLEAN map_user_stack_at(VOIDPTR pages, U32 stack_pages, user_layout_t *layout, 
                                U32 *pd, U32 vbase, U32 stack_size, TCB *proc) {
    if (!pages || !layout || !pd || !proc) return FALSE;
    if (!vbase || stack_size == 0) return FALSE;
    if (layout->stack_base == 0 || layout->stack_end == 0) return FALSE;
    if (stack_size > (layout->stack_end - layout->stack_base)) return FALSE;
    U32 start_page = (layout->stack_base - layout->bin_base) / PAGE_SIZE;
    for (U32 i = 0; i < stack_pages; i++) {
        VOIDPTR phys = (VOIDPTR)((U32)pages + (start_page + i) * PAGE_SIZE);
        MEMZERO((U32)phys, PAGE_SIZE);
        // MEMSET((U32)phys, 0xCC, PAGE_SIZE); // fill with int3 for easier debugging
        #ifdef MAP_IN_FUNCTIONS
        U32 vaddr = vbase + i * PAGE_SIZE;
        map_page(pd, vaddr, (U32)phys, PAGE_PRW);
        #endif
    }
    // Save stack top (as virtual address)
    proc->stack_phys_base = (U32 *)layout->stack_base; // physical base
    proc->stack_vtop = (U32 *)(vbase + stack_pages * PAGE_SIZE); // virtual top of stack seen by user process

    return TRUE;
}

void init_task_context(TCB *tcb, void (*entry)(void), U32 initial_state, U32 stack_size) {
    // Place the trap frame at the top of the stack
    // Stack grows down, so we subtract sizeof(TrapFrame) from the top of the stack
    // Stack top (virtual) is tcb->stack_vtop.
    // Stack base (physical) is tcb->stack_phys_base.
    // Both addresses are already set.
    U32 tf_phys_addr = (U32)tcb->stack_phys_base + 
                        (pages_from_bytes(stack_size) * PAGE_SIZE) - 
                        sizeof(TrapFrame);

    // Kernel pointer. Is already mapped and valid, pointer to physical address
    TrapFrame *k_tf = (TrapFrame *)(tf_phys_addr);

    // Initialize the trap frame
    MEMZERO(k_tf, sizeof(TrapFrame));

    k_tf->seg.ds = KDS; k_tf->seg.fs = KDS; k_tf->seg.es = KDS; k_tf->seg.gs = KDS;

    k_tf->gpr.edi = k_tf->gpr.esi = k_tf->gpr.ebp = k_tf->gpr.esp = 0;
    k_tf->gpr.ebx = k_tf->gpr.edx = k_tf->gpr.ecx = k_tf->gpr.eax = 0;

    k_tf->cpu.eip = (U32)entry;
    k_tf->cpu.cs = KCS;          // kernel CS
    k_tf->cpu.eflags = EFLAGS_IF;


    // Virtual TF pointer seen after CR3 switch:
    // This is the virtual address of the trap frame in the user process's address space
    // Already calculated as top of stack - sizeof(TrapFrame)
    tcb->tf = (TrapFrame *)((U32)tcb->stack_vtop - sizeof(TrapFrame));

    // Set initial state
    tcb->info.state = initial_state;
}


U32 *setup_user_process(TCB *proc, U8 *binary_data, U32 bin_size, U32 heap_size, U32 stack_size, U32 initial_state) {
    // Create page directory
    PD_HANDLE pdh = create_process_pagedir();
    if (!pdh.virt) return NULL;
    proc->pagedir = pdh.virt; // Virtual pointer to page directory, used by the process
    proc->pagedir_phys = pdh.phys; // Physical address of page directory, loaded into CR3 on context switch
    
    // Now we map all of kernel pages into the new page directory
    VOIDPTR kernel_pd_raw = get_page_directory();
    if(!kernel_pd_raw) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }


    // Compute memory layout
    U32 bin_pages = pages_from_bytes(bin_size);
    U32 heap_pages = pages_from_bytes(heap_size);
    U32 stack_pages = pages_from_bytes(stack_size);
    if (bin_pages == 0 || heap_pages == 0 || stack_pages == 0) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }
    proc->binary_size = bin_size;
    proc->binary_pages = bin_pages;
    proc->heap_size = heap_size;
    proc->heap_pages = heap_pages;
    proc->stack_size = stack_size;
    proc->stack_pages = stack_pages;
    U32 amount_of_pages_needed = bin_pages + heap_pages + stack_pages;
    
    VOIDPTR pages = KREQUEST_PAGES(amount_of_pages_needed);
    if (!pages) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }
    
    proc->pages = pages;
    proc->page_count = amount_of_pages_needed;

    user_layout_t layout;
    if (!compute_user_layout(pages, bin_size, heap_size, stack_size, &layout)) return NULL;
    // Now we have:
    // layout.bin_base, layout.bin_end
    // layout.heap_base, layout.heap_end
    // layout.stack_base, layout.stack_end
    //
    // All of these are presented as physical addresses and must be mapped into the new page directory
    
    // Copy kernel PDEs
    copy_kernel_pdes_with_offset(proc->pagedir_phys, kernel_pd_raw);

    
    // Map binary
    if (!map_user_binary_at(pages, bin_pages, &layout, proc->pagedir_phys, USER_BINARY_VADDR, binary_data, bin_size
    )) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }

    // Map heap. Heap is right after binary
    U32 heap_vaddr = USER_BINARY_VADDR + (layout.heap_base - layout.bin_end);
    if(!map_user_heap_at(pages, heap_pages, &layout, proc->pagedir_phys, heap_vaddr, heap_size)) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }
    // Map stack. Stack is right after heap
    // U32 stack_vaddr = heap_vaddr + (layout.stack_base - layout.heap_end);
    U32 stack_vaddr = USER_BINARY_VADDR + (layout.stack_base - layout.bin_end);
    if(!map_user_stack_at(pages, stack_pages, &layout, proc->pagedir_phys, stack_vaddr, stack_size, proc)) {
        destroy_process_pagedir(proc->pagedir_phys);
        return NULL;
    }

    // Map pages ONLY after all physical pages are allocated and binary is copied
    #ifndef MAP_IN_FUNCTIONS
    for(U32 i = 0; i < amount_of_pages_needed; i++) {
        U32 phys = (U32)pages + (i * PAGE_SIZE);
        U32 vaddr = USER_BINARY_VADDR + (( (U32)phys - layout.bin_base));
        map_page(proc->pagedir_phys, vaddr, phys, PAGE_PRW);
    }
    #endif

    // Initialize trap frame
    init_task_context(proc, (void (*)(void))USER_BINARY_VADDR, initial_state, stack_size);
    
    return proc->pagedir_phys;
}


void add_tcb_to_scheduler(TCB *new_tcb) {
    panic_if(!new_tcb, PANIC_TEXT("Cannot add null TCB to scheduler!"), PANIC_INVALID_ARGUMENT);
    
    if (new_tcb->info.state == TCB_STATE_ACTIVE) {
        active_tasks += 1;
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

BOOLEAN RUN_BINARY(U8 *proc_name, VOIDPTR file, U32 bin_size, U32 heap_size, U32 stack_size, U32 initial_state) {
    CLI;
    PIC_Mask(0);

    if(bin_size == 0 || bin_size > MAX_USER_BINARY_SIZE) {
        STI;    
        return FALSE;
    }


    TCB *new_proc = KMALLOC(sizeof(TCB));
    panic_if(!new_proc, "Unable to allocate memory for TCB!", PANIC_OUT_OF_MEMORY);
    MEMZERO(new_proc, sizeof(TCB));

    // Setup memory + trap frame
    panic_if(!setup_user_process(new_proc, (U8 *)file, bin_size, heap_size, stack_size, initial_state),
             PANIC_TEXT("Failed to set up user process"), PANIC_OUT_OF_MEMORY);

    new_proc->info.pid = get_next_pid();
    STRNCPY((char *)new_proc->info.name, (char *)proc_name, TASK_NAME_MAX_LEN);
    new_proc->info.name[TASK_NAME_MAX_LEN - 1] = '\0';

    add_tcb_to_scheduler(new_proc);
    PIC_Unmask(0); // enable IRQ0 (PIT)

    STI;
    return TRUE;
}

void remove_tcb_from_scheduler(TCB *tcb) {
    if (!tcb || tcb == &master_tcb) return;

    TCB *prev = &master_tcb;
    TCB *curr = master_tcb.next;
    while (curr != &master_tcb) {
        if (curr == tcb) {
            prev->next = curr->next;
            if (tcb->info.state == TCB_STATE_ACTIVE) {
                active_tasks--;
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}


void KILL_PROCESS(U32 pid) {
    if (pid == 0) return; // cannot kill master
    TCB *master = get_master_tcb();
    TCB *prev = master;
    TCB *curr = master->next;

    while(curr != master) {
        if(curr->info.pid == pid) {
            // Found the process to kill
            prev->next = curr->next; // remove from list

            // Free resources
            if(curr->stack_phys_base) {
                KFREE_PAGE(curr->stack_phys_base);
            }
            if(curr->pagedir) {
                // Free page directory and tables (not implemented here)
                // You would need to walk the page directory and free all user pages and tables
                destroy_process_pagedir(curr->pagedir);
            }
            remove_tcb_from_scheduler(curr);
            KFREE(curr);
            return;
            active_tasks--;
        }
        prev = curr;
        curr = curr->next;
    }
}

TCB *find_next_active_task(void) {
    if (!initialized) return NULL;

    TCB *start = current_tcb;
    TCB *next = start->next;

    while(next != start) {
        if (next->info.state == TCB_STATE_ACTIVE) {
            return next;
        }
        next = next->next;
    }

    if (start->info.state == TCB_STATE_ACTIVE) {
        return start; // only current is active
    }

    // No active tasks available
    // Fallback to master task if no other active tasks
    return &master_tcb;
}

// Called from PIT ISR to perform task switch
// Arg: current trap frame (already pushed by ISR)
// Returns: new trap frame to load (or same if no switch)
// Called from PIT ISR to perform task switch
TrapFrame* pit_handler_task_control(TrapFrame *cur) {
    // todo: tick counter here
    if (!initialized) {
        return cur;
    }

    if (current_tcb) {
        current_tcb->tf = cur;
        current_tcb->info.cpu_time += PIT_TICK_MS;
    }
    
    TCB *next = find_next_active_task();

    if (!next) {
        panic(PANIC_TEXT("No active tasks to switch to!"), PANIC_CONTEXT_SWITCH_FAILED);
        next = current_tcb; 
    }
    
    current_tcb = next;

    current_tcb->info.num_switches++;

    set_next_task_esp_val((U32)current_tcb->tf);
    set_next_task_cr3_val((U32)current_tcb->pagedir_phys);
    set_next_task_pid(current_tcb->info.pid);
    set_next_task_num_switches(current_tcb->info.num_switches);

    return current_tcb->tf;
}

void INC_rki_row(U32 *rki_row) {
    *rki_row += VBE_CHAR_HEIGHT + 2;
}
void early_debug_tcb(U32 pid) {
    // for early task debugging
    TCB *t = get_master_tcb();
    U32 rki_row = 0;
    VBE_DRAW_STRING(0, rki_row, "TCB Dump:", VBE_GREEN, VBE_BLACK);
    INC_rki_row(&rki_row);
    U8 buf[20];
    ITOA_U(active_tasks, buf, 10);
    VBE_DRAW_STRING(0, rki_row, "Active tasks:", VBE_GREEN, VBE_BLACK);
    VBE_DRAW_STRING(150, rki_row, buf, VBE_GREEN, VBE_BLACK);
    INC_rki_row(&rki_row);

    do {
        if(t->info.pid != pid) {
            if(pid != U32_MAX) continue;
        }
        INC_rki_row(&rki_row);

        VBE_DRAW_STRING(0, rki_row, "Process name:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(110, rki_row, t->info.name, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->info.pid, buf, 10);
        VBE_DRAW_STRING(0, rki_row, "TCB PID:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA(t->info.state, buf, 10);
        VBE_DRAW_STRING(0, rki_row, "State:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)t->pagedir_phys, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "Pagedir phys:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)t->pages, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "Pages:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)t->info.num_switches, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "Switches:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)t->info.cpu_time, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "CPU Time (ms):", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(150, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)get_current_task_esp(), buf, 16);
        VBE_DRAW_STRING(0, rki_row, "ESP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        ITOA_U((U32)t->tf->cpu.eip, buf, 16);
        VBE_DRAW_STRING(0, rki_row, "EIP:", VBE_GREEN, VBE_BLACK);
        VBE_DRAW_STRING(100, rki_row, buf, VBE_GREEN, VBE_BLACK);
        INC_rki_row(&rki_row);

        if(pid == U32_MAX) break;
        set_rki_row(rki_row);
        DUMP_MEMORY((U32)t->pagedir_phys, PAGE_SIZE/8);
        DUMP_MEMORY((U32)t->pages, 256);
        DUMP_MEMORY(USER_BINARY_VADDR, 256);

        VBE_UPDATE_VRAM();
    } while((t = t->next) != get_master_tcb());
}
