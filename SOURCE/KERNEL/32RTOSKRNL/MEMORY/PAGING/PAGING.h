#ifndef PAGING_H
#define PAGING_H

#include <STD/TYPEDEF.h>
#include <RTOSKRNL_INTERNAL.h>

#define PAGE_SIZE 0x1000
#define PAGE_ENTRIES 1024

#define PTE_PRESENT 0x1u
#define PTE_WRITE   0x2u
#define PTE_USER    0x4u
#define PAGE_PRESENT   PTE_PRESENT
#define PAGE_WRITE     PTE_WRITE
#define PAGE_USER      PTE_USER

// Default memory sizes for user processes
#define USER_STACK_SIZE   (8 * 1024)     // 8 KiB stack
#define USER_HEAP_SIZE    (64 * 1024)    // 64 KiB initial heap


#define USER_BINARY_BASE  0x00000000      // start of user code
#define USER_STACK_BASE   (MEM_USER_SPACE_END_MIN - USER_STACK_SIZE) // stack grows down

#define USER_VADDR_BASE   0x00000000u

/* layout offsets relative to USER_VADDR_BASE */
#define USER_BINARY_VADDR (USER_VADDR_BASE + 0x000000)  /* binary at 0 */
#define USER_HEAP_VADDR   (USER_VADDR_BASE + 0x001000)  /* will be computed */
#define USER_STACK_VADDR  (USER_VADDR_BASE + 0x00F000)  /* will be computed */



VOID INIT_PAGING(VOID);

/* Create a fresh page directory for a user process:
   - allocate it from KMALLOC (kernel heap), zero it and copy kernel PDEs
   - kernel PDEs copied without PAGE_USER flag so user-mode can't access kernel */
U32 *create_process_pagedir(void);

/* Map a single user virtual page into a process pagedir (pagedir must be kernel-allocated)
   flags: combination of PTE_WRITE (writable) — PTE_USER implied for user pages */
void MAP_USER_PAGE_IN_PAGEDIR(U32 *pagedir, VOIDPTR vaddr, VOIDPTR paddr, U32 flags);


/* Map user code (binary) at virtual addr USER_BINARY_VADDR (0x0) in given page directory */
void map_user_binary(U32 *pagedir, U8 *file_data, U32 binary_size);

/* Map user heap immediately after the binary (virtual), sized by USER_HEAP_SIZE.
   We compute the vbase dynamically so the heap begins after the binary rounded up to page. */
void map_user_heap(U32 *pagedir, U32 binary_size, U32 user_heap_size);

/* Map user stack immediately after heap in virtual space. Simple upward-growing stack.
   (If you want a downward-growing stack, allocate the virtual region and set the initial
   stack pointer appropriately in the process context.) */
void map_user_stack(U32 *pagedir, U32 binary_size, U32 user_heap_size, U32 user_stack_size, TCB *proc);

#endif // PAGING_H