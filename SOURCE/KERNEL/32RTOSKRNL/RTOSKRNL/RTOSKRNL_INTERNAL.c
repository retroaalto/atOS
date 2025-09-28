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

#define INC_rki_row(rki_row) (rki_row += VBE_CHAR_HEIGHT + 2)
#define DEC_rki_row(rki_row) (rki_row -= VBE_CHAR_HEIGHT + 2)
U32 rki_row = 0;

static inline U32 ASM_READ_ESP(void) {
    U32 esp;
    ASM_VOLATILE("mov %%esp, %0" : "=r"(esp));
    return esp;
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

static inline U32 ASM_GET_EIP(void) {
    U32 eip;
    ASM_VOLATILE("call 1f; 1: pop %0" : "=r"(eip));
    return eip;
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


static TCB master_tcb = {0};
static TCB *current_tcb = &master_tcb;
static U32 next_pid = 1; // start from 1 since 0 is master tcb

TCB *get_master_tcb(void) {
    return &master_tcb;
}
U32 get_current_pid(void) {
    return master_tcb.pid;
}
U32 get_next_pid(void) {
    return next_pid++;
}

void init_master_tcb(void) {
    master_tcb.pid = 0;
    master_tcb.state = 1; // running
    master_tcb.next = &master_tcb; // points to itself
    ASM_VOLATILE(
        "mov %%esp, %0\n\t"
        "mov %%ebp, %1\n\t"
        : "=r"(master_tcb.esp), "=r"(master_tcb.ebp)
    );
    master_tcb.stack = (U32*)master_tcb.esp;
    master_tcb.eip = 0; // not used for the master tcb
}

void init_process(TCB *proc, U32 (*entry_point)(void), U32 stack_size) {
    proc->stack = REQUEST_PAGE(1); // 4KB stack
    U32 *sp = proc->stack + 1024; // top of stack
    *(--sp) = 0x200;              // initial EFLAGS
    *(--sp) = 0x08;               // CS
    *(--sp) = (U32)entry_point;         // EIP
    proc->esp = (U32)sp;          // initial ESP
    proc->eip = (U32)entry_point; // entry point
}

void context_switch(void) {

}

void schedule_next_task(void) {
    if(current_tcb->next == &master_tcb) {
        // Only master tcb exists
        return;
    }
    current_tcb = current_tcb->next;
    while(current_tcb->state != 1) { // 1 = ready
        current_tcb = current_tcb->next;
        if(current_tcb == &master_tcb) {
            // No ready tasks, stay in master tcb
            current_tcb = &master_tcb;
            return;
        }
    }
}

void pit_handler_task_control(void) {
    // schedule_next_task();
    // context_switch();
}






VOIDPTR LOAD_KERNEL_SHELL(VOID) {
    VBE_FLUSH_SCREEN();
    // Loads the kernel shell from disk into memory
    // First cdrom read is tried, if no cdrom is found, we try to read from the first ata drive

    U8 filename[] = "PROGRAMS/TEST1.BIN"; // ISO9660 format
    IsoDirectoryRecord *fileptr = ISO9660_FILERECORD_TO_MEMORY((CHAR*)filename);
    if(!fileptr) {
        panic("PANIC: Failed to read kernel shell from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return NULLPTR;
    }

    VOIDPTR FILEDATA = ISO9660_READ_FILEDATA_TO_MEMORY(fileptr);
    if(!FILEDATA) {
        ISO9660_FREE_MEMORY(fileptr);
        panic("PANIC: Failed to read kernel shell data from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return NULLPTR;
    }
    ISO9660_FREE_MEMORY(fileptr);
    return FILEDATA;
}


BOOLEAN RUN_KERNEL_SHELL(VOIDPTR addr) {
    if(!addr) return FALSE;
    void (*entry)(void) = (void(*)(void))addr;
    entry();

    return TRUE;
}

void LOAD_AND_RUN_KERNEL_SHELL(VOID) {
    VOIDPTR file = LOAD_KERNEL_SHELL();
    if(!file) {
        panic("PANIC: Failed to load kernel shell!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return;
    }

    if(!RUN_KERNEL_SHELL(file)) {
        panic("PANIC: Failed to run kernel shell!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return;
    }
    

    VBE_DRAW_STRING(0, rki_row, "Kernel shell loaded and running!", PANIC_COLOUR);
    VBE_UPDATE_VRAM();
}