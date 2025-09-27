#include <STD/MEM.h>
#include <STD/STRING.h>
#include <VIDEO/VESA.h>
#include <VIDEO/VBE.h>
#include <PAGING/PAGEFRAME.h>


#define PAGE_PRESENT   0x1
#define PAGE_WRITE     0x2
#define PAGE_USER      0x4

// aligned to 4KiB
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
__attribute__((aligned(4096))) U32 page_directory[PAGE_DIRECTORY_SIZE];
__attribute__((aligned(4096))) U32 page_table_0[PAGE_TABLE_SIZE];


VOID INIT_PAGING() {
    // Clear directory
    for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) page_directory[i] = 0;
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) page_table_0[i] = 0;

    // Virtual 0x00000000 → Physical 0x00200000
    page_table_0[0] = (0x00200000) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    // Point directory[0] to our page table
    page_directory[0] = ((U32)page_table_0) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    // Load into CR3
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging
    U32 cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}