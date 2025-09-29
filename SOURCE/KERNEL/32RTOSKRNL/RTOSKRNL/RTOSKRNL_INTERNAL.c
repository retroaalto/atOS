#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <STD/ASM.h>
#include <MEMORY/PAGING/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <DRIVERS/DISK/ATA_ATAPI.h>
#include <FS/ISO9660/ISO9660.h>
#include <FS/FAT32/FAT32.h>
#include <STD/STRING.h>
#include <MEMORY/KMALLOC/KMALLOC.h>
#include <DRIVERS/PIT/PIT.h>

#define INC_rki_row(rki_row) (rki_row += VBE_CHAR_HEIGHT + 2)
#define DEC_rki_row(rki_row) (rki_row -= VBE_CHAR_HEIGHT + 2)
U32 rki_row = 0;

static inline U32 ASM_READ_EBP(void) {
    U32 ebp;
    ASM_VOLATILE("mov %%ebp, %0" : "=r"(ebp));
    return ebp;
}

static inline U32 ASM_READ_ESP(void) {
    U32 esp;
    ASM_VOLATILE("mov %%esp, %0" : "=r"(esp));
    return esp;
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

static inline regs ASM_READ_REGS(void) {
    regs regs;
    ASM_VOLATILE("mov %%eax, %0" : "=r"(regs.eax));
    ASM_VOLATILE("mov %%ebx, %0" : "=r"(regs.ebx));
    ASM_VOLATILE("mov %%ecx, %0" : "=r"(regs.ecx));
    ASM_VOLATILE("mov %%edx, %0" : "=r"(regs.edx));
    ASM_VOLATILE("mov %%esi, %0" : "=r"(regs.esi));
    ASM_VOLATILE("mov %%edi, %0" : "=r"(regs.edi));
    ASM_VOLATILE("mov %%ebp, %0" : "=r"(regs.ebp));
    ASM_VOLATILE("mov %%esp, %0" : "=r"(regs.esp));
    return regs;
}

#define PANIC_COLOUR VBE_WHITE, VBE_BLUE

void DUMP_CALLER_STACK(U32 count) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Caller stack dump:", PANIC_COLOUR);
    INC_rki_row(rki_row);
    U32 *ebp;
    ASM_VOLATILE("mov %%ebp, %0" : "=r"(ebp));
    for (U32 i = 0; i < count && ebp; i++) {
        ITOA(ebp[1], buf, 16); // return address
        ebp = (U32*)ebp[0];    // next EBP
        VBE_DRAW_STRING(0, rki_row, buf, PANIC_COLOUR);
        INC_rki_row(rki_row);
    }

    VBE_UPDATE_VRAM();
}

void DUMP_STACK(U32 esp, U32 count) {
    U32 *stack = (U32*)esp;
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Stack dump:", PANIC_COLOUR);
    INC_rki_row(rki_row);
    for(U32 i = 0; i < count; i++) {
        ITOA((U32)(stack[i]), buf, 16);
        VBE_DRAW_STRING(0, rki_row, buf, PANIC_COLOUR);
        INC_rki_row(rki_row);
    }
    VBE_UPDATE_VRAM();
}

void DUMP_REGS(regs *r) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "EAX: ", PANIC_COLOUR);
    ITOA(r->eax, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "EBX: ", PANIC_COLOUR);
    ITOA(r->ebx, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "ECX: ", PANIC_COLOUR);
    ITOA(r->ecx, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "EDX: ", PANIC_COLOUR);
    ITOA(r->edx, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "ESI: ", PANIC_COLOUR);
    ITOA(r->esi, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "EDI: ", PANIC_COLOUR);
    ITOA(r->edi, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "EBP: ", PANIC_COLOUR);
    ITOA(r->ebp, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "ESP: ", PANIC_COLOUR);
    ITOA(r->esp, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}

void DUMP_ERRCODE(U32 errcode) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Error code: ", PANIC_COLOUR);
    ITOA(errcode, buf, 16);
    VBE_DRAW_STRING(100, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}

void DUMP_INTNO(U32 int_no) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Interrupt number: ", PANIC_COLOUR);
    ITOA(int_no, buf, 16);
    VBE_DRAW_STRING(200, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}

void DUMP_MEMORY(U32 addr, U32 length) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Memory dump:", PANIC_COLOUR);
    INC_rki_row(rki_row);
    U8 *ptr = (U8*)addr;
    for(U32 i = 0; i < length; i++) {
        if(i % 16 == 0) {
            if(i != 0) INC_rki_row(rki_row);
            ITOA((U32)(ptr + i), buf, 16);
            VBE_DRAW_STRING(0, rki_row, buf, PANIC_COLOUR);
        }
        ITOA(ptr[i], buf, 16);
        VBE_DRAW_STRING(100 + (i % 16) * 30, rki_row, buf, PANIC_COLOUR);
        INC_rki_row(rki_row);
    }
    VBE_UPDATE_VRAM();
}

void panic(const U8 *msg, U32 errmsg) {
    VBE_CLEAR_SCREEN(VBE_BLUE);
    U8 buf[16];
    ITOA(errmsg, buf, 16);
    VBE_DRAW_STRING(0, rki_row, "ERRORCODE: 0x", PANIC_COLOUR);
    VBE_DRAW_STRING(VBE_CHAR_WIDTH*13, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, msg, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "System halted, Dump as in panic() call.", PANIC_COLOUR);
    INC_rki_row(rki_row);
    U32 esp = ASM_READ_ESP();
    U32 eip = ASM_GET_EIP();
    
    INC_rki_row(rki_row);    
    VBE_DRAW_STRING(0, rki_row, "Registers:", PANIC_COLOUR);
    INC_rki_row(rki_row);
    regs r = ASM_READ_REGS();
    DUMP_REGS(&r);
    VBE_DRAW_STRING(0, rki_row, "EIP:", PANIC_COLOUR);
    ITOA(eip, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "ESP:", PANIC_COLOUR);
    ITOA(esp, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);

    INC_rki_row(rki_row);
    DUMP_STACK(esp, 10);
    
    INC_rki_row(rki_row);
    DUMP_CALLER_STACK(10);
    INC_rki_row(rki_row);

    VBE_DRAW_STRING(0, rki_row, "Memory dump at EIP", PANIC_COLOUR);
    INC_rki_row(rki_row);
    
    DUMP_MEMORY(eip, 64);
    VBE_UPDATE_VRAM();
    ASM_VOLATILE("cli; hlt");
}

void panic_if(BOOL condition, const U8 *msg, U32 errmsg) {
    if (condition) {
        panic(msg, errmsg);
    }
}


// static TCB master_tcb = {0};
// static TCB *current_tcb = &master_tcb;
// static U32 next_pid = 1; // start from 1 since 0 is master tcb

// TCB *get_current_tcb(void) {
//     return current_tcb;
// }
// TCB *get_master_tcb(void) {
//     return &master_tcb;
// }
// U32 get_current_pid(void) {
//     return master_tcb.pid;
// }
// U32 get_next_pid(void) {
//     // TODO_FIXME: handle pid wraparound and reuse of freed pids, though not critical now
//     if(next_pid == 0xFFFFFFFF) next_pid = 2; // wrap around, avoid 0. 1 is usually SHELL
//     return next_pid++;
// }


// static inline U32 read_cr3(void) {
//     U32 val;
//     asm volatile("mov %%cr3, %0" : "=r"(val));
//     return val;
// }
// static inline void write_cr3(U32 val) {
//     asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
// }

// void add_tcb_to_scheduler(TCB *new_tcb) {
//     TCB *master = get_master_tcb();
//     TCB *last = master;
//     while(last->next != master) last = last->next;

//     last->next = new_tcb;
//     new_tcb->next = master;
//     new_tcb->state = TCB_STATE_ACTIVE;
// }


// void init_master_tcb(void) {
//     master_tcb.pid = 0;
//     master_tcb.state = TCB_STATE_ACTIVE; // running
//     master_tcb.next = &master_tcb; // circular list
//     ASM_VOLATILE(
//         "mov %%esp, %0\n\t"
//         "mov %%ebp, %1\n\t"
//         : "=r"(master_tcb.esp), "=r"(master_tcb.ebp)
//     );
//     master_tcb.stack = (U32*)master_tcb.esp;
//     master_tcb.eip = 0; // not used
// }


// void init_process(TCB *proc, U32 (*entry_point)(void), U32 stack_size) {
//     /* proc->stack should be the virtual base (lowest virtual addr of stack region).
//        For downward-growing stack initial ESP should be base + stack_size_in_bytes. */
//     U32 stack_pages = pages_from_bytes(stack_size);
//     U32 top_of_stack = (U32)proc->stack + stack_pages * PAGE_SIZE;

//     /* push initial frame as if coming from an interrupt/iret */
//     U32 *sp = (U32 *)top_of_stack;
//     *(--sp) = 0x200;                // initial EFLAGS
//     *(--sp) = 0x1B;                 // SS (user data selector with RPL=3) if using rings
//     *(--sp) = top_of_stack;         // initial ESP (user)
//     *(--sp) = 0x23;                 // CS (user code selector with RPL=3)
//     *(--sp) = (U32)entry_point;     // EIP

//     proc->esp = (U32)sp;            // initial kernel-visible saved ESP for later swapping
//     proc->eip = (U32)entry_point;
// }


// U32 *setup_user_process(TCB *proc, U8 *binary_data, U32 binary_size, U32 user_heap_size, U32 user_stack_size) {
    
//     U32 *pd = create_process_pagedir();
//     if (!pd) return NULL;
//     proc->pagedir = pd;
    
//     /* map and copy binary */
//     map_user_binary(pd, binary_data, binary_size);

//     /* map heap and init header in physical page */
//     map_user_heap(pd, binary_size, user_heap_size);

//     /* compute virtual heap start and store it in TCB (virtual addr) */
//     U32 binary_pages = pages_from_bytes(binary_size);
//     U32 heap_vbase = USER_BINARY_VADDR + binary_pages * PAGE_SIZE;
//     proc->user_heap_start = (USER_HEAP_BLOCK *)heap_vbase;

//     /* map stack and set proc->stack (virtual base) */
//     map_user_stack(pd, binary_size, user_heap_size, user_stack_size, proc);

//     return pd;
// }





// void schedule_next_task(void) {
//     if(current_tcb->next == &master_tcb) {
//         // Only master tcb exists
//         return;
//     }
//     current_tcb = current_tcb->next;
//     while(current_tcb->state != TCB_STATE_ACTIVE) {
//         current_tcb = current_tcb->next;
//         if(current_tcb == &master_tcb) {
//             // No ready tasks, stay in master tcb
//             current_tcb = &master_tcb;
//             return;
//         }
//     }
// }

// void context_switch(TCB *next) {
//     current_tcb->esp = ASM_READ_ESP();
//     current_tcb->ebp = ASM_READ_EBP();

//     // Switch page directory
//     write_cr3((U32)next->pagedir);

//     // Switch stack pointers
//     ASM_SET_ESP(next->esp);
//     ASM_SET_EBP(next->ebp);

//     // Update current TCB
//     current_tcb = next;
// }



// void pit_handler_task_control(void) {
//     TCB *prev = current_tcb;
//     schedule_next_task();
//     if(current_tcb != prev) {
//         context_switch(current_tcb);
//     }
// }


// void init_multitasking(void) {
//     init_master_tcb();
//     PIT_INIT();

//     // Test requesting and freeing a page, should not panic
//     VOIDPTR page = REQUEST_PAGE();
//     panic_if(!page, "PANIC: Unable to request page!", PANIC_OUT_OF_MEMORY);
//     FREE_PAGE(page);
// }

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





// BOOLEAN LOAD_KERNEL_SHELL(VOIDPTR *file_data_out, U32 *bin_size_out) {

//     U8 filename[] = "PROGRAMS/TEST1.BIN"; // ISO9660 format
//     IsoDirectoryRecord *fileptr = ISO9660_FILERECORD_TO_MEMORY((CHAR*)filename);
//     if(!fileptr) {
//         panic("PANIC: Failed to read kernel shell from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
//         return FALSE;
//     }

//     *bin_size_out = fileptr->extentLengthLE;
//     *file_data_out = ISO9660_READ_FILEDATA_TO_MEMORY(fileptr);

//     if(!*file_data_out) {
//         ISO9660_FREE_MEMORY(fileptr);
//         panic("PANIC: Failed to read kernel shell data from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
//         return FALSE;
//     }

//     ISO9660_FREE_MEMORY(fileptr);
//     return TRUE;
// }



// void LOAD_AND_RUN_KERNEL_SHELL(VOID) {
//     VOIDPTR file = NULLPTR;
//     U32 bin_size = 0;

//     if(!LOAD_KERNEL_SHELL(&file, &bin_size)) {
//         panic("PANIC: Failed to load kernel shell!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
//         return;
//     }

//     U8 buf[50];
//     ITOA(GET_RESERVED_RAM(), buf, 16);
//     VBE_DRAW_STRING(0, rki_row, "Free memory before running kernel shell: 0x", PANIC_COLOUR);
//     VBE_DRAW_STRING(VBE_CHAR_WIDTH*40, rki_row, buf, PANIC_COLOUR);
//     INC_rki_row(rki_row);
//     VBE_UPDATE_VRAM();


//     for (int j = 0; j < 10; j++) {
//         VOIDPTR page = REQUEST_PAGE();
//         if (!page) break; // no more pages
//         // optionally, print page index for debug
//     }   
//     panic("PANIC: Stopping before running kernel shell to avoid page fault!", 0x0);
//     // RUN_BINARY(file, bin_size, USER_HEAP_SIZE, USER_STACK_SIZE);
    
//     VBE_DRAW_STRING(0, rki_row, "Kernel shell loaded and running!", PANIC_COLOUR);
//     VBE_UPDATE_VRAM();
// }