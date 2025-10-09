/*
Internal RTOSKRNL functions. Some important functions like panic() are here,
and some not-so-important functions are here.
*/
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <STD/ASM.h>
#include <MEMORY/PAGEFRAME/PAGEFRAME.h>
#include <MEMORY/PAGING/PAGING.h>
#include <DRIVERS/DISK/ATA_ATAPI.h>
#include <FS/ISO9660/ISO9660.h>
#include <FS/FAT32/FAT32.h>
#include <STD/STRING.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <CPU/PIT/PIT.h>
#include <ACPI/ACPI.h>
#include <CPU/ISR/ISR.h> // for regs struct
#include <PROC/PROC.h>
#include <VIDEO/VBE.h>
#include <DRIVERS/PS2/KEYBOARD.h>


#define INC_rki_row(rki_row) (rki_row += VBE_CHAR_HEIGHT + 2)
#define DEC_rki_row(rki_row) (rki_row -= VBE_CHAR_HEIGHT + 2)
static U32 rki_row __attribute__((section(".data"))) = 0;

void set_rki_row(U32 row) {
    rki_row = row;
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
#define PANIC_DEBUG_COLOUR VBE_RED, VBE_LIGHT_CYAN

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
    ITOA_U(errcode, buf, 16);
    VBE_DRAW_STRING(100, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}

void DUMP_INTNO(U32 int_no) {
    U8 buf[20];
    VBE_DRAW_STRING(0, rki_row, "Interrupt number: ", PANIC_COLOUR);
    ITOA_U(int_no, buf, 16);
    VBE_DRAW_STRING(200, rki_row, buf, PANIC_COLOUR);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}

void DUMP_MEMORY(U32 addr, U32 length) {
    U8 buf[20];
    U8 *ptr = (U8*)addr;
    for(U32 i = 0; i < length; i++) {
        // Start new line every 16 bytes
        if(i % 16 == 0) {
            if(i != 0) INC_rki_row(rki_row);  // move down only at new line
            ITOA((U32)(ptr + i), buf, 16);
            VBE_DRAW_STRING(0, rki_row, buf, PANIC_COLOUR);
        }

        // Print the byte
        ITOA(ptr[i], buf, 16);
        VBE_DRAW_STRING(100 + (i % 16) * 30, rki_row, buf, PANIC_COLOUR);
    }

    // After dumping, move to next line
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
}



void panic_reg(regs *r, const U8 *msg, U32 errmsg) {
    CLI;
    VBE_COLOUR fg = VBE_WHITE;
    VBE_COLOUR bg = VBE_BLUE;
    VBE_CLEAR_SCREEN(bg);
    U8 buf[16];
    ITOA(errmsg, buf, 16);
    VBE_DRAW_STRING(0, rki_row, "ERRORCODE: 0x", fg, bg);
    // VBE_UPDATE_VRAM();
    // HLT;
    VBE_DRAW_STRING(VBE_CHAR_WIDTH*13, rki_row, buf, fg, bg);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, msg, fg, bg);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "System halted, Dump as of exception raise.", fg, bg);
    INC_rki_row(rki_row);
    U32 esp = r->esp;
    U32 ebp = r->ebp;
    
    INC_rki_row(rki_row);    
    VBE_DRAW_STRING(0, rki_row, "Registers:", fg, bg);
    INC_rki_row(rki_row);
    DUMP_REGS(r);
    INC_rki_row(rki_row);
    
    INC_rki_row(rki_row);
    DUMP_CALLER_STACK(10);
    INC_rki_row(rki_row);

    VBE_DRAW_STRING(0, rki_row, "Stack dump at ESP, starting from ESP-60", fg, bg);
    INC_rki_row(rki_row);
    DUMP_STACK(esp - 60, 80);
    INC_rki_row(rki_row);
    VBE_UPDATE_VRAM();
    ASM_VOLATILE("cli; hlt");
}
void PANIC_RAW(const U8 *msg, U32 errmsg, VBE_COLOUR fg, VBE_COLOUR bg) {
    CLI;
    rki_row = 0;
    VBE_CLEAR_SCREEN(bg);
    U8 buf[16];
    ITOA(errmsg, buf, 16);
    VBE_DRAW_STRING(0, rki_row, "ERRORCODE: 0x", fg, bg);
    // VBE_UPDATE_VRAM();
    // HLT;
    VBE_DRAW_STRING(VBE_CHAR_WIDTH*13, rki_row, buf, fg, bg);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, msg, fg, bg);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "System halted, Dump as in panic() call.", fg, bg);
    INC_rki_row(rki_row);
    U32 esp = ASM_READ_ESP();
    U32 eip = ASM_GET_EIP();
    
    INC_rki_row(rki_row);    
    VBE_DRAW_STRING(0, rki_row, "Registers:", fg, bg);
    INC_rki_row(rki_row);
    regs r = ASM_READ_REGS();
    DUMP_REGS(&r);
    VBE_DRAW_STRING(0, rki_row, "EIP:", fg, bg);
    ITOA(eip, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, fg, bg);
    INC_rki_row(rki_row);
    VBE_DRAW_STRING(0, rki_row, "ESP:", fg, bg);
    ITOA(esp, buf, 16);
    VBE_DRAW_STRING(50, rki_row, buf, fg, bg);
    INC_rki_row(rki_row);

    INC_rki_row(rki_row);
    DUMP_STACK(esp, 10);
    
    INC_rki_row(rki_row);
    DUMP_CALLER_STACK(10);
    INC_rki_row(rki_row);

    VBE_DRAW_STRING(0, rki_row, "Memory dump at EIP", fg, bg);
    INC_rki_row(rki_row);
    
    DUMP_MEMORY(eip, 64);

    VBE_DRAW_STRING(0, rki_row, "Memory dump at error code.", fg, bg);
    INC_rki_row(rki_row);
    DUMP_MEMORY(errmsg, 256);

    VBE_UPDATE_VRAM();
    ASM_VOLATILE("cli; hlt");
}

void panic(const U8 *msg, U32 errmsg) {
    PANIC_RAW(msg, errmsg, PANIC_COLOUR);
}
void panic_if(BOOL condition, const U8 *msg, U32 errmsg) {
    if (condition) {
        panic(msg, errmsg);
    }
}

void panic_debug(const U8 *msg, U32 errmsg) {
    PANIC_RAW(msg, errmsg, PANIC_DEBUG_COLOUR);
}
void panic_debug_if(BOOL condition, const U8 *msg, U32 errmsg) {
    if (condition) {
        panic_debug(msg, errmsg);
    }
}

void system_halt(VOID) {
    ASM_VOLATILE("cli; hlt");
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
}

void system_halt_if(BOOL condition) {
    if (condition) {
        system_halt();
    }
}

void system_reboot(VOID) {
    ASM_VOLATILE(
        "cli\n"             // Disable interrupts
        "mov $0xFE, %al\n"  // Reset command
        "out %al, $0x64\n"  // Send to keyboard controller
        "hlt\n"             // Stop CPU if reset fails
    );
}
void system_reboot_if(BOOL condition) {
    if (condition) {
        system_reboot();
    }
}
void system_shutdown(VOID) {
    panic(PANIC_TEXT("System shutdown not implemented, here's a halt instead"), PANIC_NONE);   
    ACPI_SHUTDOWN_SYSTEM();
    // If ACPI shutdown fails, halt the system
    system_halt();
}
void system_shutdown_if(BOOL condition) {
    if (condition) {
        system_shutdown();
    }
}

// assert function
void assert(BOOL condition) {
    if (!condition) {
        panic(PANIC_TEXT("Assertion failed"), PANIC_UNKNOWN_ERROR);
    }
}









// #define SHELL_PATH "INNER/INNER2/INSIDE_1.TXT"
// #define DEBUG_PRINT_SHELL_CONTENTS_AND_HALT

#define SHELL_PATH "PROGRAMS/atOShell/atOShell.BIN"
// #define SHELL_PATH "PROGRAMS/TEST1/TEST1.BIN"

VOIDPTR LOAD_KERNEL_SHELL(U32 *bin_size_out, IsoDirectoryRecord **fileptr_out) {
    U8 filename[] = SHELL_PATH; // ISO9660 format
    *fileptr_out = ISO9660_FILERECORD_TO_MEMORY((CHAR*)filename);
    if(!*fileptr_out) {
        panic("PANIC: Failed to read kernel shell from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return NULL;
    }
    if(bin_size_out) 
        *bin_size_out = (*fileptr_out)->extentLengthLE;


    VOIDPTR file_data_out = ISO9660_READ_FILEDATA_TO_MEMORY(*fileptr_out);

    if(!file_data_out) {
        ISO9660_FREE_MEMORY(*fileptr_out);
        panic("PANIC: Failed to read kernel shell data from disk!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return NULL;
    }
    return file_data_out;
}



void LOAD_AND_RUN_KERNEL_SHELL(VOID) {
    VBE_FLUSH_SCREEN();
    VOIDPTR file = NULLPTR;
    static U32 bin_size = 0;
    IsoDirectoryRecord *fileptr = NULLPTR;

    if(!(file = LOAD_KERNEL_SHELL(&bin_size, &fileptr))) {
        panic("PANIC: Failed to load kernel shell!", PANIC_KERNEL_SHELL_GENERAL_FAILURE);
        return;
    }

    panic_if(
        !RUN_BINARY("atOShell", file, bin_size, USER_HEAP_SIZE, USER_STACK_SIZE, 
            TCB_STATE_IMMORTAL | TCB_STATE_INFO_CHILD_PROC_HANDLER , 0), 
        "PANIC: Failed to run kernel shell!", 
        PANIC_KERNEL_SHELL_GENERAL_FAILURE
    );
    ISO9660_FREE_MEMORY(file);
    ISO9660_FREE_MEMORY(fileptr);
}


void RTOSKRNL_LOOP(VOID) {
    VBE_DRAW_ELLIPSE(1000, 0, 200,200, VBE_AQUA);
    U32 i = 0;
    U32 j = 0;
    U32 pass = 0;
    TCB *a = get_tcb_by_pid(1);
    // DUMP_MEMORY(a->framebuffer_phys + (PAGE_SIZE * 5), PAGE_SIZE);
    while(1) {
        // VBE_DRAW_ELLIPSE(500, 500, 200, 200, VBE_RED);
        VBE_DRAW_LINE(0, i, j, i, pass++ % 2 == 0 ? VBE_RED : VBE_GREEN);
        i++;
        if(i >= 1024) i = 0;
        j++;
        if(j >= 768) j = 0;
        if(pass >= 100) {
            pass = 0;
            pass = (pass % 2 == 0) ? 1 : -1;
        }
        handle_kernel_messages();
    }
}