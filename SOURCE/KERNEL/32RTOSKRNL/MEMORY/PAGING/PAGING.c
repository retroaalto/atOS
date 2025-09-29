#include <STD/MEM.h>
#include <STD/STRING.h>
#include <STD/ASM.h>
#include <MEMORY/MEMORY.h>   // provides MEM_USER_SPACE_BASE, etc.
#include <PAGING/PAGEFRAME.h> // REQUEST_PAGE(), etc., used later
#include <PAGING/PAGING.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h> // panic()
#include <VIDEO/VBE.h>       // VBE_DRAW_STRING(), etc., used in panic
#include <STD/BINARY.h>

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE     1024
#define PAGE_ALIGN __attribute__((aligned(4096)))
#define FOUR_MB (0x400000u)
#define PAGE_SIZE (0x1000u)
